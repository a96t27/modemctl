#include <transport.h>

#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <parser.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <response.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <libgen.h>
#include <modem.h>
#include <modem_lookup.h>
#include <context.h>

int get_ttyusb_info(const char *dev_path, size_t dev_path_len, struct usb_info *result)
{
        if (dev_path == NULL || result == NULL) {
                return EXIT_FAILURE;
        }
        struct usb_info usb_info = { 0 };
        char dev_path_cpy[PATH_MAX] = { 0 };
        snprintf(dev_path_cpy, sizeof(dev_path_cpy), "%.*s", dev_path_len, dev_path); // basename may change original string

        char *dev_name;
        dev_name = basename(dev_path_cpy);

        char base_path[PATH_MAX] = { 0 };
        snprintf(base_path, sizeof(base_path), "/sys/class/tty/%s/device/..", dev_name); // Symbolic links are friends we found along the way.

        char file_path[PATH_MAX] = { 0 };
        FILE *file = NULL;
        int count = 0;

        snprintf(file_path, sizeof(file_path), "%s/bInterfaceNumber", base_path);

        file = fopen(file_path, "r");
        if (file == NULL) {
                return EXIT_FAILURE;
        }
        count = fscanf(file, "%" SCNx8, &(usb_info.interface));
        fclose(file);
        if (count != 1) {
                return EXIT_FAILURE;
        };
        snprintf(file_path, sizeof(file_path), "%s/../bConfigurationValue", base_path);

        file = fopen(file_path, "r");
        if (file == NULL) {
                return EXIT_FAILURE;
        }
        count = fscanf(file, "%" SCNu8, &(usb_info.configuration));
        fclose(file);
        if (count != 1) {
                return EXIT_FAILURE;
        };

        snprintf(file_path, sizeof(file_path), "%s/../idVendor", base_path);
        file = fopen(file_path, "r");
        if (file == NULL) {
                return EXIT_FAILURE;
        }
        count = fscanf(file, "%" SCNx16, &(usb_info.vendor_id));
        fclose(file);
        if (count != 1) {
                return EXIT_FAILURE;
        };

        snprintf(file_path, sizeof(file_path), "%s/../idProduct", base_path);
        file = fopen(file_path, "r");
        if (file == NULL) {
                return EXIT_FAILURE;
        }
        count = fscanf(file, "%" SCNx16, &(usb_info.product_id));
        fclose(file);
        if (count != 1) {
                return EXIT_FAILURE;
        };
        *result = usb_info;
        return EXIT_SUCCESS;
}

int get_at_port(const char *dev_path, size_t dev_path_len, struct at_port *result)
{
        if (dev_path == NULL || result == NULL) {
                return EXIT_FAILURE;
        }
        struct at_port at_port = { 0 };
        if (get_ttyusb_info(dev_path, dev_path_len, &at_port.usb_info) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
        }
        if (!is_supported_device(&(at_port.usb_info))) {
                return EXIT_FAILURE;
        }
        at_port.fd = open(dev_path, O_RDWR);
        if (at_port.fd < 0) {
                return EXIT_FAILURE;
        }
        *result = at_port;
        return EXIT_SUCCESS;
}

bool is_supported_device(const struct usb_info *usb_info)
{
        if (usb_info == NULL) {
                return false;
        }
        return is_supported_modem(usb_info->vendor_id, usb_info->product_id) && usb_info->interface == SUPPORTED_INTERFACE;
}

int setup_at_port(struct at_port *port)
{
        if (port == NULL) {
                return EXIT_FAILURE;
        }
        struct termios options = { 0 };
        if (tcgetattr(port->fd, &options) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
        }

        cfmakeraw(&options);

        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_cflag |= (CLOCAL | CREAD);

        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 50;

        if (tcsetattr(port->fd, TCSANOW, &options) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
        }

        port->retry_max = MODEMCTL_DEFAULT_RETRY_MAX;
        port->timeout_seconds = MODEMCTL_DEFAULT_TIMEOUT;

        return EXIT_SUCCESS;
}

int find_any_at_port(struct at_port *result)
{
        if (result == NULL) {
                return EXIT_FAILURE;
        }
        char dev_root[] = "/dev";

        struct dirent *root_de = NULL;
        DIR *root_dp = opendir(dev_root);
        if (root_dp == NULL) {
                return EXIT_FAILURE;
        }

        char file_path[PATH_MAX] = { 0 };
        struct at_port at_port = { 0 };
        int ret = EXIT_FAILURE;
        int len = 0;
        while ((root_de = readdir(root_dp)) != NULL) {
                len = snprintf(file_path, sizeof(file_path), "%s/%s", dev_root, root_de->d_name);
                if (len < 0 || (size_t)len > sizeof(file_path)) {
                        continue;
                }
                if (get_at_port(file_path, len, &at_port) == EXIT_SUCCESS) {
                        ret = EXIT_SUCCESS;
                        *result = at_port;
                        break;
                }
        }
        closedir(root_dp);
        return ret;
}


struct cJSON *at_execute(struct context *ctx, const char *cmd, size_t cmd_len)
{
        if (ctx->port == NULL || cmd == NULL || cmd_len < 2) {
                return NULL;
        }
        struct cJSON *result = cJSON_CreateArray();
        if (result == NULL) {
                return NULL;
        }
        tcflush(ctx->port->fd, TCIFLUSH);
        write(ctx->port->fd, cmd, cmd_len);
        const char cmd_end[] = "\r\n";
        write(ctx->port->fd, cmd_end, sizeof(cmd_end) - 1);
        char buf[SERIAL_IO_BUF_MAX] = { 0 };
        size_t len = 0;
        ssize_t bytes = 0;
        bool success = false;
        while (len < sizeof(buf) && (bytes = read(ctx->port->fd, &buf[len], 1)) > 0) {
                if (bytes == 0 && ctx->running != NULL && *ctx->running) {
                        success = false;
                        break;
                }
                len += bytes;
                if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
                        buf[len - 2] = '\0';
                        buf[len - 1] = '\0';
                        len -= 2;
                        struct cJSON *str_json = cJSON_CreateString(buf);
                        if (str_json == NULL) {
                                break;
                        }
                        cJSON_AddItemToArray(result, str_json);
                        if (is_error(buf, len)) {
                                success = false;
                                break;
                        }

                        if (is_ok(buf, len)) {
                                success = true;
                                break;
                        }
                        len = 0;
                }
        }

        struct cJSON *data = cJSON_CreateObject();
        if (data == NULL) {
                cJSON_Delete(result);
                return NULL;
        }
        cJSON_AddItemToObject(data, "result", result);
        snprintf(buf, sizeof(buf), "%.*s", cmd_len, cmd);
        cJSON_AddStringToObject(data, "command", cmd);
        char type[] = "at";
        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        int msg_len = 0;
        if (success) {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Successfully executed '%.*s' command", cmd_len, cmd);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to execute '%.*s' command", cmd_len, cmd);
        }
        struct cJSON *resp = create_response(
                success,
                type, sizeof(type) - 1,
                msg_buf, msg_len,
                data
        );

        return resp;
}