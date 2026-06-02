#ifndef MODEM_H
#define MODEM_H

#include <stdbool.h>
#include <inttypes.h>
#include <cjson/cJSON.h>
#include <inttypes.h>

#define LF "\n"
#define CR "\r"
#define CRLF "\r\n"

enum action_code {
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
        int (*print_parser_resp) (struct cJSON *parser_output);
        const char *at_cmd;
};

struct modem {
        uint16_t vendor_id;
        uint16_t product_id;
        struct action actions[__ACTIONS_MAX];

};

bool is_action_implemented(struct action *action);

#endif