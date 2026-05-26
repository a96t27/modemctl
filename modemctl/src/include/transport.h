#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#define SUPPORTED_VENDOR_ID 0x2C7C
#define SUPPORTED_PRODUCT_ID 0x0306
#define SUPPORTED_INTERFACE 2
#define SERIAL_IO_BUF_MAX 4096

#define MODEMCTL_DEFAULT_RETRY_MAX 3
#define MODEMCTL_DEFAULT_TIMEOUT 3.0

struct usb_info {
        uint16_t vendor_id;
        uint16_t product_id;
        uint8_t configuration;
        uint8_t interface;
};

struct at_port {
        int fd;
        unsigned int retry_max;
        unsigned int retry_count;
        float timeout_seconds;
        struct usb_info usb_info;
};

int get_ttyusb_info(const char *dev_path, size_t dev_path_len, struct usb_info *result);
int get_at_port(const char *dev_path, size_t dev_path_len, struct at_port *result);
int find_any_at_port(struct at_port *result);
int setup_at_port(struct at_port *port);
bool is_supported_device(const struct usb_info *usb_info);
struct cJSON *at_execute(struct at_port *port, const char *cmd, size_t cmd_len);

#endif