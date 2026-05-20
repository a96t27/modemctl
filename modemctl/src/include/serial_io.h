#ifndef SERIAL_IO_H
#define SERIAL_IO_H

#include <modemctl_context.h>
#include <stddef.h>
#include <cjson/cJSON.h>

#define SERIAL_IO_BUF_MAX 1024

int at_find_port(void);
int at_get_port(const char *path);
int at_setup_port(int fd);
struct cJSON *at_execute(struct ModemctlContext *ctx, char *cmd, size_t cmd_len);

#endif