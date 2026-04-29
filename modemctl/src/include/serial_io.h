#ifndef SERIAL_IO_H
#define SERIAL_IO_H

int at_get_port(void);
int at_setup_port(int fd);
int at_execute(int fd, int cmd_size, char *cmd, int output_buf_size, char *output_buf);

#endif