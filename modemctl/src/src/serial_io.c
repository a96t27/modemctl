#include <serial_io.h>
#include <errno.h>
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


int at_get_port(const char *path)
{
        if (path == NULL) {
                return -1;
        }
        int fd = open(path, O_RDWR); // TODO: automatinė paieška

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


// struct text_line *at_execute(int fd, int cmd_size, char *cmd)
// {
//         if (fd < 0 || cmd_size < 2 || cmd == NULL) {
//                 return NULL;
//         }
//         tcflush(fd, TCIFLUSH);
//         write(fd, cmd, cmd_size - 1);
//         const char cmd_end[] = "\r\n";
//         write(fd, cmd_end, sizeof(cmd_end));
//         int n = 0;
//         int bytes = 0;
//         char output_buf[TEXT_LINE_MAX];
//         struct text_line lines = { 0 };
//         struct text_line *last_line = &lines;
//         while ((bytes = read(fd, &output_buf[n], 1)) > 0 && n + 1 < sizeof(output_buf)) {
//                 n += bytes;
//                 output_buf[n] = '\0';
//                 if (output_buf[n - 1] == '\r') {
//                         n--;
//                         output_buf[n] = '\0';
//                 } else if (output_buf[n - 1] == '\n') {
//                         n--;
//                         output_buf[n] = '\0';
//                         last_line->next = create_line(n, output_buf);
//                         last_line = last_line->next;
//                         n = 0;
//                         if (is_end(last_line)) {
//                                 break;
//                         }
//                 }
//         }

//         return lines.next;
// }

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
                                break;
                        }
                        len = 0;
                }
        }

        struct cJSON *data = cJSON_CreateObject();
        if (data == NULL) {
                return NULL;
        }
        cJSON_AddItemToObject(data, "result", result);
        snprintf(buf, sizeof(buf), "%.*s", cmd_len, cmd);
        cJSON_AddStringToObject(data, "command", cmd);
        char type[] = "at";
        char message[] = "Successfully executed AT command"; // TODO: make better message generation
        struct cJSON *resp = create_response(
                true,
                type, sizeof(type) - 1,
                message, sizeof(message) - 1,
                data
        );

        return resp;
}