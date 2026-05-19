#ifndef PARSER_H
#define PARSER_H

#include <cjson/cJSON.h>
#include <stddef.h>
#include <stdbool.h>

bool is_empty(const char *line, size_t len);
bool is_ok(const char *line, size_t len);
bool is_error(const char *line, size_t len);
bool is_end(const char *line, size_t len);
bool is_unsolicited_event(const char *line, size_t len);
struct cJSON *parse_get_imei(struct cJSON *at_resp);

#endif