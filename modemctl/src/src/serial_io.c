#include <serial_io.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>



int at_get_port(void)
{
        int fd = open("/dev/ttyUSB2", O_RDWR);

        return fd;
}

int at_setup_port(int fd)
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
        options.c_cc[VTIME] = 20;
        tcsetattr(fd, TCSANOW, &options);
        return EXIT_SUCCESS;
}


int at_execute(int fd, int cmd_size, char *cmd, int output_buf_size, char *output_buf)
{
        if (fd < 0 || cmd_size < 2 || output_buf_size < 2 || cmd == NULL || output_buf == NULL) {
                return EXIT_FAILURE;
        }
        // memset(output_buf, 0, output_buf_size);
        tcflush(fd, TCIFLUSH);
        write(fd, cmd, cmd_size);
        const char cmd_end[] = "\r\n";
        write(fd, cmd_end, sizeof(cmd_end));
        int n = 0;
        int bytes = 0;
        while ((bytes = read(fd, &output_buf[n], 1)) > 0) {
                n += bytes;
                output_buf[n] = '\0';
                if (strstr(output_buf, "OK") || strstr(output_buf, "ERROR")) // TODO: reik lygint tik buferio pabaiga
                        break;

        }

        return EXIT_SUCCESS;
}
