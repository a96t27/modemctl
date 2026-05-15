#ifndef COMMANDS_H
#define COMMANDS_H

struct cJSON *create_response(char *method, int success, char *message, struct cJSON *data);
struct cJSON *get_imei(int fd);

#endif