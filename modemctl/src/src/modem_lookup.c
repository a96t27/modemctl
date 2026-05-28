#include <modem_lookup.h>
#include <modem.h>
#include <modems/quectel_eg06.h>
#include <stdbool.h>
#include <transport.h>



struct modem *modems[] = {
        &quectel_eg06,
};
size_t modems_count = sizeof(modems) / sizeof(modems[0]);

int get_modem(uint16_t vendor_id, uint16_t product_id, struct modem *result)
{
        if (result == NULL) {
                return EXIT_FAILURE;
        }
        struct modem *m = NULL;
        for (int i = 0; i < modems_count; i++) {
                m = modems[i];
                if (m == NULL) {
                        continue;
                }
                if (m->vendor_id == vendor_id && m->product_id == product_id) {
                        *result = *m;
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

bool is_supported_modem(uint16_t vendor_id, uint16_t product_id)
{
        struct modem m = { 0 };
        return get_modem(vendor_id, product_id, &m) == EXIT_SUCCESS;
}