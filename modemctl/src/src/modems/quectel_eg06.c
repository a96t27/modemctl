#include <modems/quectel_eg06.h>


#include <stdlib.h>
#include <cjson/cJSON.h>
#include <transport.h>
#include <parser.h>
#include <response.h>
#include <string.h>
#include <stdio.h>

static int quectel_eg06_parse_imei(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = cJSON_GetObjectItemCaseSensitive(at_resp, "data");
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *at_command = cJSON_GetObjectItemCaseSensitive(at_data, "command");
        char *cmd = cJSON_GetStringValue(at_command);
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        char *imei = NULL;
        size_t imei_len = 0;
        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue; // gal reiketu iseiti su klaida?
                }
                char *line = cJSON_GetStringValue(item);
                size_t len = strlen(line);
                if (is_ok(line, len)) {
                        break;
                }
                if (is_error(line, len)) {
                        success = false;
                        break;
                }
                if (is_empty(line, strlen(line))
                        || strcmp(line, cmd) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
                imei_len = len;
                imei = line;
        }
        if (imei == NULL || imei_len == 0) {
                success = false;
        }
        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "imei";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "IMEI: %.*s", imei_len, imei);
                cJSON_AddStringToObject(data, "imei", imei);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to read IMEI");
                type = "error";
                cJSON_Delete(data);
                data = NULL;
        }
        if (msg_len > (typeof(msg_len))sizeof(msg_buf) - 1) {
                msg_len = (typeof(msg_len))sizeof(msg_buf) - 1;
        }
        *resp = create_response(success, type, strlen(type), msg_buf, msg_len, data);
        return EXIT_SUCCESS;
}

struct modem quectel_eg06 = {
        .vendor_id = 0x2C7C,
        .product_id = 0x0306,
        .actions = {
                [GET_IMEI] = {
                        .parser = quectel_eg06_parse_imei,
                        .at_cmd = "AT+GSN",
                },
        },
};