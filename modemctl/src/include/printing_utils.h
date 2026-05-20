#ifndef PRINTING_UTILS_H
#define PRINTING_UTILS_H

#include <cjson/cJSON.h>

int print_at_resp(struct cJSON *at_resp);
int print_responses(struct cJSON *responses);

#endif