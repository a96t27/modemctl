#ifndef MODEM_LOOKUP_H
#define MODEM_LOOKUP_H

#include <inttypes.h>
#include <modem.h>
#include <stdbool.h>

#define USB_ID(vendor_id, product_id) (((uint32_t)(vendor_id) << 16)|(uint16_t)(product_id))
#define USB_VID(usb_id) ((uint16_t)((usb_id) >> 16))
#define USB_PID(usb_id) ((uint16_t)((usb_id) & 0x0000FFFF))

bool is_supported_modem(uint16_t vendor_id, uint16_t product_id);
int get_modem(uint16_t vendor_id, uint16_t product_id, struct modem *result);

#endif