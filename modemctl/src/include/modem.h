#ifndef MODEM_H
#define MODEM_H

#include <stdbool.h>
#include <inttypes.h>
#include <cjson/cJSON.h>

#include <transport.h>
#include <inttypes.h>

#define AT_CMD_MAX 128


enum action_code {
        NO_ACTION,
        GET_IMEI,
        GET_MODEL,
        GET_OPERATOR,
        GET_CONNECTION_STATUS,
        GET_BAND,
        GET_SIM_STATUS,
        GET_SERVING_CELL,
        GET_NEIGHBOUR_CELL,
        GET_SIGNAL,
        GET_IP_ADDRESS,
        GET_TEMPERATURE,
        GET_CURRENT_APN,
        GET_PHONE_NUMBER,
        GET_SMS,
        __ACTIONS_MAX
};

struct action {
        int (*parser) (struct cJSON *at_output, struct cJSON **resp);
        char at_cmd[AT_CMD_MAX];
};

struct modem {
        uint16_t vendor_id;
        uint16_t product_id;
        struct action actions[__ACTIONS_MAX];
};

#endif