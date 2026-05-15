#ifndef PARSER_H
#define PARSER_H

#include <text_lines.h>
#include <cjson/cJSON.h>

int is_empty(struct text_line *text_line);
int is_ok(struct text_line *text_line);
int is_error(struct text_line *text_line);
int is_end(struct text_line *text_line);
struct text_line *skip_empty(struct text_line *text_lines);
struct cJSON *parse_get_imei(struct text_line *text_lines);

#endif