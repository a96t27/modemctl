#ifndef MODEM_H
#define MODEM_H

#include <stdbool.h>
#include <inttypes.h>
#include <cjson/cJSON.h>

#include <transport.h>
#include <inttypes.h>
#include <modems/quectel_eg06.h>


#define AT_CMD_MAX 128

#define USB_ID(vendor_id, product_id) (((uint32_t)(vendor_id) << 16)|(uint16_t)(product_id))
#define USB_VID(usb_id) ((uint16_t)((usb_id) >> 16))
#define USB_PID(usb_id) ((uint16_t)((usb_id) & 0x0000FFFF))

enum action_code {
        NO_ACTION,
        GET_IMEI,
        GET_STATUS,
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



// int(*actions[__ACTIONS_MAX]) (struct at_port *port, struct cJSON *req, struct cJSON **resp);
#endif