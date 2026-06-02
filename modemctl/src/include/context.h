#ifndef CONTEXT_H
#define CONTEXT_H

#include <cjson/cJSON.h>
#include <modem.h>
#include <stdbool.h>

struct usb_info {
        uint16_t vendor_id;
        uint16_t product_id;
        uint8_t configuration;
        uint8_t interface;
};

struct at_port {
        int fd;
        unsigned int retry_max;
        float timeout_seconds;
        struct usb_info usb_info;
};

struct context {
        struct at_port *port;
        struct modem *modem;
        bool debug;
        bool json;
        bool *running;
};

bool is_valid_context(struct context *ctx);
bool is_valid_at_port(struct at_port *port);

#endif