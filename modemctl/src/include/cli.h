#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <cjson/cJSON.h>
#include <linux/limits.h>

struct arguments {
        bool all;
        bool imei;
        bool opertaor;
        bool signal;
        bool cell;
        bool band;
        bool status;
        bool json;
        char device[PATH_MAX];
        bool debug;
        bool watch;
        struct cJSON *at_cmds; // array of strings
};

int parse_arguments(int argc, char **argv, struct arguments *args);
int print_at_resp(struct cJSON *at_resp);
int print_responses(struct cJSON *responses);
int print_at_resp(struct cJSON *at_resp);
int print_responses(struct cJSON *responses);

#endif