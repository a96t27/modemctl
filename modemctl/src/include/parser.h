#ifndef PARSER_H
#define PARSER_H

#include <cjson/cJSON.h>
#include <stddef.h>
#include <stdbool.h>

bool is_empty(const char *line, size_t len);
bool is_ok(const char *line, size_t len);
bool is_error(const char *line, size_t len);
bool is_end(const char *line, size_t len);
// struct text_line *skip_empty(struct text_line *text_lines);
// struct cJSON *parse_get_imei(struct text_line *text_lines);

#endif