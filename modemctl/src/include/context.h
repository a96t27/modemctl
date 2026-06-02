#ifndef CONTEXT_H
#define CONTEXT_H

#include <transport.h>
#include <cjson/cJSON.h>
#include <modem.h>
#include <stdbool.h>

struct context {
        struct at_port *port;
        struct modem *modem;
        bool debug;
        bool json;
};

bool is_valid_context(struct context *ctx);

#endif