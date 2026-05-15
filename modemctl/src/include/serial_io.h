#ifndef SERIAL_IO_H
#define SERIAL_IO_H

struct ModemSettings {
        int CMEE;
};


int at_get_port(char *path);
int at_setup_port(int fd);
struct text_line *at_execute(int fd, int cmd_size, char *cmd);

#endif