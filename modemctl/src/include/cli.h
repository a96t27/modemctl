#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <cjson/cJSON.h>
#include <linux/limits.h>
#include <modem.h>

struct arguments {
        bool actions[__ACTIONS_MAX];
        bool all;
        bool json;
        char device[PATH_MAX];
        bool debug;
        bool watch;
        struct cJSON *at_cmds; // array of strings
};

enum {
        DEVICE_KEY = 1000,
        DEBUG_KEY,
        JSON_KEY,
        AT_KEY,
        ALL_KEY,
        WATCH_KEY,
        IMEI_KEY,
        MODEL_KEY,
        OPERATOR_KEY,
        CONNECTION_KEY,
        BAND_KEY,
        SIM_KEY,
        SERVING_CELL_KEY,
        NEIGHBOUR_CELLS_KEY,
        SIGNAL_KEY,
        IP_KEY,
        APN_KEY,
        NUMBER_KEY,
        TEMPERATURE_KEY,
        SMS_KEY,
};

int parse_arguments(int argc, char **argv, struct arguments *args);
int print_at_resp(struct cJSON *at_resp);
int print_responses(struct cJSON *responses);
int print_at_resp(struct cJSON *at_resp);
int print_responses(struct cJSON *responses);

#endif