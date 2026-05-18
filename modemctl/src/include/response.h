#ifndef RESPONSE_H
#define RESPONSE_H

#include <cjson/cJSON.h>
#include <stdbool.h>

#define RESPONSE_BUFFER_SIZE 1024

struct cJSON *create_response(bool success, const char *type, size_t type_len, const char *message, size_t message_len, struct cJSON *data);
bool is_valid_response(struct cJSON *resp);

#endif