#include <serial_io.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <parser.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <modemctl_context.h>
#include <response.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <inttypes.h>

#define USB_DEV_PATH "/sys/bus/usb/devices"
#define USB_ID(vendor_id, product_id) (((uint32_t)(vendor_id) << 16)|(uint16_t)(product_id))
#define USB_VID(usb_id) ((uint16_t)((usb_id) >> 16))
#define USB_PID(usb_id) ((uint16_t)((usb_id) & 0x0000FFFF))
#define SUPPORTED_USB_ID USB_ID(0x2C7C, 0x0306)
#define DEFAULT_USB_INTERFACE 2

int read_id(const char *bus_name, size_t bus_name_len, const char *filename, size_t filename_len)
{
        if (bus_name == NULL || filename == NULL) {
                return -1;
        }
        char buf[PATH_MAX] = { 0 };
        snprintf(buf, sizeof(buf), "%s/%.*s/%.*s", USB_DEV_PATH, bus_name_len, bus_name, filename_len, filename);
        FILE *file = fopen(buf, "r");
        if (file == NULL) {
                return -1;
        }
        uint16_t id = 0;
        int ret = -1;
        if (fscanf(file, "%" SCNx16, &id) == 1) {
                ret = id;
        }
        fclose(file);
        return ret;
}

int at_find_port(void)
{
        int fd = -1;
        char root_path[] = USB_DEV_PATH;
        struct dirent *root_de = NULL;
        DIR *root_dp = opendir(root_path);
        if (root_dp == NULL) {
                return -1;
        }
        char buf[PATH_MAX] = { 0 };
        while (fd < 0 && (root_de = readdir(root_dp)) != NULL) {
                char id_vendor[] = "idVendor";
                int vid = read_id(root_de->d_name, strlen(root_de->d_name), id_vendor, sizeof(id_vendor) - 1);
                char id_product[] = "idProduct";
                int pid = read_id(root_de->d_name, strlen(root_de->d_name), id_product, sizeof(id_product) - 1);
                char configuration_value[] = "bConfigurationValue";
                int conf_id = read_id(root_de->d_name, strlen(root_de->d_name), configuration_value, sizeof(configuration_value) - 1);
                if (pid < 0 || vid < 0 || conf_id < 0) {
                        continue;
                }
                uint32_t usb_id = USB_ID(vid, pid);
                if (usb_id != SUPPORTED_USB_ID) {
                        continue;
                }
                snprintf(buf, sizeof(buf), "%s/%s/%s:%d.%d", root_path, root_de->d_name, root_de->d_name, conf_id, DEFAULT_USB_INTERFACE);
                struct dirent *bus_de = NULL;
                DIR *bus_dp = opendir(buf);
                if (bus_dp == NULL) {
                        continue;
                }
                while (fd < 0 && (bus_de = readdir(bus_dp)) != NULL) {
                        int dev_n = 0;
                        if (sscanf(bus_de->d_name, "ttyUSB%d", &dev_n) != 1) {
                                continue;
                        }
                        snprintf(buf, sizeof(buf), "/dev/%s", bus_de->d_name);
                        fd = at_get_port(buf);
                }
                closedir(bus_dp);

        }
        closedir(root_dp);
        return fd;
}

int at_get_port(const char *path)// TODO: automatinė paieška
{
        if (path == NULL) {
                return -1;
        }
        int fd = open(path, O_RDWR);
        return fd;
}

int at_setup_port(int fd) // TODO: galimai reikia paleisti pora komandu konfiguravimui
{
        if (fd < 0) {
                return EXIT_FAILURE;

        }
        struct termios options;
        tcgetattr(fd, &options);

        cfsetispeed(&options, B19200);
        cfsetospeed(&options, B19200);
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_cflag |= (CLOCAL | CREAD);

        cfmakeraw(&options);
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 50;
        tcsetattr(fd, TCSANOW, &options);
        return EXIT_SUCCESS;
}


struct cJSON *at_execute(struct ModemctlContext *ctx, char *cmd, size_t cmd_len) // TODO: improve error handling (use goto)
{
        if (ctx == NULL || ctx->fd < 0 || cmd == NULL || cmd_len < 2) {
                return NULL;
        }
        struct cJSON *result = cJSON_CreateArray();
        if (result == NULL) {
                return NULL;
        }
        tcflush(ctx->fd, TCIFLUSH);
        write(ctx->fd, cmd, cmd_len);
        const char cmd_end[] = "\r\n";
        write(ctx->fd, cmd_end, sizeof(cmd_end) - 1);
        char buf[SERIAL_IO_BUF_MAX] = { 0 };
        size_t len = 0;
        ssize_t bytes = 0;
        bool success = false;
        while (len < sizeof(buf) && (bytes = read(ctx->fd, &buf[len], 1)) > 0) {
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
                        if (is_end(buf, len)) {
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