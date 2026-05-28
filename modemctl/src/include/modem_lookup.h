#ifndef MODEM_LOOKUP_H
#define MODEM_LOOKUP_H

#include <inttypes.h>
#include <modem.h>
#include <stdbool.h>

bool is_supported_modem(uint16_t vendor_id, uint16_t product_id);
int get_modem(uint16_t vendor_id, uint16_t product_id, struct modem *result);

#endif