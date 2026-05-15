#include <serial_io.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <text_lines.h>
#include <parser.h>
#include <stdio.h>


int at_get_port(char *path)
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


struct text_line *at_execute(int fd, int cmd_size, char *cmd)
{
        if (fd < 0 || cmd_size < 2 || cmd == NULL) {
                return NULL;
        }
        tcflush(fd, TCIFLUSH);
        write(fd, cmd, cmd_size - 1);
        const char cmd_end[] = "\r\n";
        write(fd, cmd_end, sizeof(cmd_end));
        int n = 0;
        int bytes = 0;
        char output_buf[TEXT_LINE_MAX];
        struct text_line lines = { 0 };
        struct text_line *last_line = &lines;
        while ((bytes = read(fd, &output_buf[n], 1)) > 0 && n + 1 < sizeof(output_buf)) {
                n += bytes;
                output_buf[n] = '\0';
                if (output_buf[n - 1] == '\r') {
                        n--;
                        output_buf[n] = '\0';
                } else if (output_buf[n - 1] == '\n') {
                        n--;
                        output_buf[n] = '\0';
                        last_line->next = create_line(n, output_buf);
                        last_line = last_line->next;
                        n = 0;
                        if (is_end(last_line)) {
                                break;
                        }
                }
        }

        return lines.next;
}
