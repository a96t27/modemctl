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
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *at_command = cJSON_GetObjectItemCaseSensitive(at_data, "command");
        char *cmd = cJSON_GetStringValue(at_command);
        size_t cmd_len = strlen(cmd);

        struct cJSON *resp_data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        char *imei = NULL;
        size_t imei_len = 0;

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
                imei_len = len;
                imei = line;
        }
        if (imei == NULL || imei_len == 0) {
                success = false;
        }

        char type[] = "imei";

        if (success) {
                char msg[] = "Successfully got IMEI";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, resp_data);
                cJSON_AddStringToObject(resp_data, "imei", imei);
        } else {
                char msg[] = "Failed to get IMEI";
                cJSON_Delete(resp_data);
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, resp_data);
        }

        return EXIT_SUCCESS;
}

static int quectel_eg06_print_imei(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *imei = cJSON_GetObjectItemCaseSensitive(data, "imei");
        printf("IMEI: %s\n", cJSON_GetStringValue(imei));
        return EXIT_SUCCESS;
}

static int quectel_eg06_parse_model(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *at_command = cJSON_GetObjectItemCaseSensitive(at_data, "command");
        char *cmd = cJSON_GetStringValue(at_command);
        size_t cmd_len = strlen(cmd);
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;

        struct cJSON *vendor = NULL;
        struct cJSON *model = NULL;
        struct cJSON *revision = NULL;

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
                if (vendor == NULL) {
                        vendor = cJSON_AddStringToObject(data, "vendor", line);
                } else if (model == NULL) {
                        model = cJSON_AddStringToObject(data, "model", line);
                } else if (revision == NULL) {
                        revision = cJSON_AddStringToObject(data, "revision", line);
                }
        }
        if (vendor == NULL || model == NULL || revision == NULL) {
                success = false;
        }
        char type[] = "model";

        if (success) {
                char msg[] = "Successfully got model";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else {
                char msg[] = "Failed to get model";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }

        return EXIT_SUCCESS;
}


static int quectel_eg06_print_model(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *vendor = cJSON_GetObjectItemCaseSensitive(data, "vendor");
        struct cJSON *model = cJSON_GetObjectItemCaseSensitive(data, "model");
        struct cJSON *revision = cJSON_GetObjectItemCaseSensitive(data, "revision");
        if (!cJSON_IsString(vendor) || !cJSON_IsString(model) || !cJSON_IsString(revision)) {
                return EXIT_FAILURE;
        }
        printf("Model: %s %s %s\n", cJSON_GetStringValue(vendor), cJSON_GetStringValue(model), cJSON_GetStringValue(revision));
        return EXIT_SUCCESS;
}



static int quectel_eg06_parse_operator(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = false;
        char operator_id[17] = { 0 }; // TODO: constant

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (sscanf(line, "+COPS: %*d,%*d,\"%16[^\"]\"", operator_id) == 1) {
                        cJSON_AddStringToObject(data, "operator", operator_id);
                        success = true;
                }
        }

        char type[] = "operator";
        if (success) {
                char msg[] = "Successfully got operator";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else {
                char msg[] = "Failed to get operator";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}


static int quectel_eg06_print_operator(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *operator = cJSON_GetObjectItemCaseSensitive(data, "operator");
        if (!cJSON_IsString(operator)) {
                return EXIT_FAILURE;
        }
        printf("Operator: %s\n", cJSON_GetStringValue(operator));
        return EXIT_SUCCESS;
}

char *connection_status_txt[] = {
        "Disconnected",
        "Connected",
        "Connecting",
        "Unknown",
        "Registration denied",
        "Connected (roaming)",
};

static int quectel_eg06_parse_connection_status(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        int connection_status = 4; // UNKNOWN
        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (sscanf(line, "+CREG: %*d,%d", &connection_status) == 1) {
                        continue;
                }
        }
        char type[] = "connection_status";
        if (connection_status < 0 || connection_status >(int)sizeof(connection_status)) {
                success = false;
        }

        if (success) {
                char msg[] = "Successfully got connection status";
                cJSON_AddStringToObject(data, "connection_text", connection_status_txt[connection_status]);
                cJSON_AddNumberToObject(data, "connection_code", (double)connection_status);
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get connection status";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        }

        return EXIT_SUCCESS;
}

static int quectel_eg06_print_connection_status(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *connection_status = cJSON_GetObjectItemCaseSensitive(data, "connection_text");
        if (!cJSON_IsString(connection_status)) {
                return EXIT_FAILURE;
        }
        printf("Connection: %s\n", cJSON_GetStringValue(connection_status));
        return EXIT_SUCCESS;
}

struct band {
        uint64_t mask;
        const char *name;
};

struct band wcdma_bands[] = {
        {
                .mask = 0x00000010,
                .name = "WCDMA 2100",
        },
        {
                .mask = 0x00000020,
                .name = "WCDMA 1900",
        },
        {
                .mask = 0x00000040,
                .name = "WCDMA 850",
        },
        {
                .mask = 0x00000080,
                .name = "WCDMA 900",
        },
        {
                .mask = 0x00000100,
                .name = "WCDMA 800",
        },
        {
                .mask = 0x00000200,
                .name = "WCDMA 1700",
        },
        {
                .mask = 0x00000800,
                .name = "WCDMA 1800",
        },
        {
                .mask = 0x00001000,
                .name = "WCDMA Japan 850"
        },
};

size_t wcdma_bands_len = sizeof(wcdma_bands) / sizeof(wcdma_bands[0]);

struct band lte_bands[] = {
        {
                .mask = 0x1,
                .name = "LTE B1",
        },
        {
                .mask = 0x4,
                .name = "LTE B3",
        },
        {
                .mask = 0x10,
                .name = "LTE B5",
        },
        {
                .mask = 0x40,
                .name = "LTE B7"
        },
        {
                .mask = 0x80,
                .name = "LTE B8",
        },
        {
                .mask = 0x80000,
                .name = "LTE B20",
        },
};
size_t lte_bands_len = sizeof(lte_bands) / sizeof(lte_bands[0]);

static int quectel_eg06_parse_band(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        uint32_t bandval = 0;
        uint64_t ltebandval = 0;
        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (sscanf(line, "+QCFG: \"band\",0x%" SCNx32 ",0x%" SCNx64, &bandval, &ltebandval) == 1) {
                        continue;
                }
        }

        struct cJSON *bands_json = cJSON_CreateArray();
        cJSON_AddItemToObject(data, "bands", bands_json);


        for (int i = 0;i < (int)wcdma_bands_len; i++) {
                struct band *b = &wcdma_bands[i];
                if (!(b->mask & bandval)) {
                        continue;
                }
                cJSON_AddItemToArray(bands_json, cJSON_CreateString(b->name));
        }

        for (int i = 0;i < (int)lte_bands_len; i++) {
                struct band *b = &lte_bands[i];
                if (!(b->mask & ltebandval)) {
                        continue;
                }
                cJSON_AddItemToArray(bands_json, cJSON_CreateString(b->name));
        }
        char type[] = "band";

        if (success) {
                char msg[] = "Successfully got bands";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get connection status";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}

static int quectel_eg06_print_band(struct cJSON *parser_output)
{

        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *bands = cJSON_GetObjectItemCaseSensitive(data, "bands");
        if (!cJSON_IsArray(bands)) {
                return EXIT_FAILURE;
        }
        struct cJSON *item = NULL;
        printf("Bands: ");
        bool first = true;
        cJSON_ArrayForEach(item, bands)
        {
                if (!cJSON_IsString(item)) {
                        continue;
                }
                if (!first) {
                        printf(", ");
                }
                first = false;
                printf("%s", cJSON_GetStringValue(item));
        }
        printf("\n");
        return EXIT_SUCCESS;
}

char *sim_state_txt[] = {
        "removed",
        "inserted",
        "unkown",
};

static int quectel_eg06_parse_sim_status(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        int sim_state = 2; // Unknown
        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                sscanf(line, "+QSIMSTAT: %*d,%d", &sim_state);

        }

        if (sim_state < 0 || sim_state >= (int)sizeof(sim_state_txt)) {
                success = false;
        }
        char type[] = "sim_state";

        if (success) {
                char msg[] = "Successfully got bands";
                cJSON_AddStringToObject(data, "state", sim_state_txt[sim_state]);
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get connection status";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}

static int quectel_eg06_print_sim_status(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *state = cJSON_GetObjectItemCaseSensitive(data, "state");
        if (!cJSON_IsString(state)) {
                return EXIT_FAILURE;
        }
        printf("SIM: %s\n", cJSON_GetStringValue(state));
        return EXIT_SUCCESS;
}

static int quectel_eg06_parse_signal(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        int signal_strength = 99; // Unknown
        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                sscanf(line, "+CSQ: %d,%*d", &signal_strength);
        }
        char strength_str[16];
        if (signal_strength == 0) {
                strncpy(strength_str, "<-113dBm", sizeof(strength_str) - 1);
        } else if (signal_strength == 1) {
                strncpy(strength_str, "-111dBm", sizeof(strength_str) - 1);
        } else if (signal_strength >= 2 && signal_strength <= 30) {
                int str_dBm = ((-53 - (-109)) / (30 - 2)) * signal_strength - 109;
                snprintf(strength_str, sizeof(strength_str), "%ddBm", str_dBm);
        } else if (signal_strength == 31) {
                strncpy(strength_str, ">-51dBm", sizeof(strength_str) - 1);
        } else {
                strncpy(strength_str, "unknown", sizeof(strength_str) - 1);
        }

        char type[] = "signal";

        if (success) {
                char msg[] = "Successfully got signal strength";
                cJSON_AddStringToObject(data, "signal", strength_str);
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get signal strength";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}


static int quectel_eg06_print_signal(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *signal = cJSON_GetObjectItemCaseSensitive(data, "signal");
        if (!cJSON_IsString(signal)) {
                return EXIT_FAILURE;
        }
        printf("Signal: %s\n", cJSON_GetStringValue(signal));
        return EXIT_SUCCESS;
}

static int quectel_eg06_parse_temperature(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        int t1 = 0;
        int t2 = 0;
        int t3 = 0;
        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                sscanf(line, "+QTEMP: %d,%d,%d", &t1, &t2, &t3);
        }
        double avg_temp = (t1 + t2 + t3) / 3.0;

        char type[] = "temperature";

        if (success) {
                char msg[] = "Successfully got temperature";
                cJSON_AddNumberToObject(data, "temperature", avg_temp);
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get temperature";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}

static int quectel_eg06_print_temperature(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *temperature = cJSON_GetObjectItemCaseSensitive(data, "temperature");
        if (!cJSON_IsNumber(temperature)) {
                return EXIT_FAILURE;
        }
        printf("Temperature: %.01f\n", cJSON_GetNumberValue(temperature));
        return EXIT_SUCCESS;
}

static int quectel_eg06_parse_apn(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;
        char apn[64] = { 0 };
        struct cJSON *apns = cJSON_CreateArray();
        cJSON_AddItemToObject(data, "apns", apns);

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (sscanf(line, "+CGDCONT: %*d,\"%*[^\"]\",\"%63[^\"]\"", apn) == 1) {
                        cJSON_AddItemToArray(apns, cJSON_CreateString(apn));
                }
        }

        char type[] = "apn";

        if (success) {
                char msg[] = "Successfully got APN";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get apns";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}

static int quectel_eg06_print_apn(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *apns = cJSON_GetObjectItemCaseSensitive(data, "apns");
        if (!cJSON_IsArray(apns)) {
                return EXIT_FAILURE;
        }
        printf("APNS: ");
        bool first = true;
        struct cJSON *item = NULL;
        cJSON_ArrayForEach(item, apns)
        {
                if (!cJSON_IsString(item)) {
                        continue;
                }
                if (!first) {
                        printf(", ");
                }
                first = false;
                printf("%s", cJSON_GetStringValue(item));
        }
        printf("\n");
        return EXIT_SUCCESS;
}

static int quectel_eg06_parse_serving_cell(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = false;

        char state[16] = { 0 };
        char generation[16] = { 0 };
        uint64_t id = 0;

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (sscanf(line, "+QENG: \"servingcell\",\"%15[^\"]\",\"%15[^\"]\",%*[^,],%*[^,],%*[^,],%llx", state, generation, &id) == 3) {
                        cJSON_AddStringToObject(data, "generation", generation);
                        cJSON_AddStringToObject(data, "state", state);
                        cJSON_AddNumberToObject(data, "id", id);
                        success = true;
                }
        }

        char type[] = "serving_cell";

        if (success) {
                char msg[] = "Successfully got serving cell";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get serving cell";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}

static int quectel_eg06_print_serving_cell(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *generation = cJSON_GetObjectItemCaseSensitive(data, "generation");
        struct cJSON *state = cJSON_GetObjectItemCaseSensitive(data, "state");
        struct cJSON *id = cJSON_GetObjectItemCaseSensitive(data, "id");
        if (!cJSON_IsString(generation) || !cJSON_IsString(state) || !cJSON_IsNumber(id)) {
                return EXIT_FAILURE;
        }
        printf("Serving cell: %s %s %.00f\n", cJSON_GetStringValue(generation), cJSON_GetStringValue(state), cJSON_GetNumberValue(id));
        return EXIT_SUCCESS;
}

enum qeng_mode {
        QENG_UNKNOWN,
        QENG_LTE,
        QENG_WCDMA,
};

static int quectel_eg06_parse_neighbour_cells(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;

        enum qeng_mode mode = QENG_UNKNOWN;
        char mode_buf[8] = { 0 };
        const char lte[] = "LTE";
        const char wcdma[] = "WCDMA";

        char technology[8];
        char id[16];

        struct cJSON *cells = cJSON_CreateArray();
        cJSON_AddItemToObject(data, "cells", cells);
        struct cJSON *cell = NULL;

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (mode == QENG_UNKNOWN && sscanf(line, "+QENG: \"servingcell\",%*[^,],\"%7[^\"]", mode_buf) == 1) {
                        if (strcmp(mode_buf, lte) == 0) {
                                mode = QENG_LTE;
                        } else if (strcmp(mode_buf, wcdma) == 0) {
                                mode = QENG_WCDMA;
                        }
                } else if (sscanf(line, "+QENG: \"neighbour%*[^\"]\",\"%7[^\"]\",%*[^,],%15[^,],", technology, id) == 2) {
                        cell = cJSON_CreateObject();
                        cJSON_AddItemToArray(cells, cell);
                        cJSON_AddStringToObject(cell, "technology", technology);
                        if (mode == QENG_LTE) {
                                cJSON_AddStringToObject(cell, "physical_cell_id", id);
                        } else if (mode == QENG_WCDMA && strcmp(technology, "LTE") == 0) {
                                cJSON_AddStringToObject(cell, "cell_id", id);
                        }
                }
        }

        char type[] = "neighbour_cells";

        if (success) {
                char msg[] = "Successfully got neighbour cells";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get neighbour cells";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}


static int quectel_eg06_print_neighbour_cells(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *cells = cJSON_GetObjectItemCaseSensitive(data, "cells");
        if (!cJSON_IsArray(cells)) {
                return EXIT_FAILURE;
        }
        struct cJSON *item = NULL;
        printf("Neighbour cells:\n");
        cJSON_ArrayForEach(item, cells)
        {
                if (!cJSON_IsObject(item)) {
                        continue;
                }
                struct cJSON *technology = cJSON_GetObjectItemCaseSensitive(item, "technology");
                struct cJSON *phys_id = cJSON_GetObjectItemCaseSensitive(item, "physical_cell_id");
                struct cJSON *cell_id = cJSON_GetObjectItemCaseSensitive(item, "cell_id");
                if (!cJSON_IsString(technology)) {
                        continue;
                }
                printf("  - RAT = %s", cJSON_GetStringValue(technology));
                if (cJSON_IsString(phys_id)) {
                        printf(" physical_cell_id = %s", cJSON_GetStringValue(phys_id));
                }
                if (cJSON_IsString(cell_id)) {
                        printf(" cell_id = %s", cJSON_GetStringValue(cell_id));
                }
                printf("\n");
        }
        return EXIT_SUCCESS;
}


char *message_type[] = {
        "unread",
        "read",
        "unsent",
        "sent",
        "all",
};

static int quectel_eg06_parse_sms(struct cJSON *at_resp, struct cJSON **resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *at_data = get_response_data(at_resp);
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        struct cJSON *data = cJSON_CreateObject();
        struct cJSON *item = NULL;
        bool success = true;

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        size_t msg_len = 0;

        char beginning[] = "SMS:\n";
        strncpy(msg_buf, beginning, sizeof(msg_buf));
        msg_len += sizeof(beginning) - 1;

        struct cJSON *msgs = cJSON_CreateArray();
        cJSON_AddItemToObject(data, "messages", msgs);

        int index = 0;
        int stat = 0;
        int length = 0;

        cJSON_ArrayForEach(item, at_result)
        {
                if (!cJSON_IsString(item)) {
                        continue;
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
                if (sscanf(line, "+CMGL: %d,%d,,%d", &index, &stat, &length) == 3) { // <alpha> may occur
                        struct cJSON *sms = cJSON_CreateObject();
                        cJSON_AddItemToArray(msgs, sms);
                        cJSON_AddNumberToObject(sms, "index", index);
                        cJSON_AddStringToObject(sms, "stat", message_type[stat]);
                        cJSON_AddNumberToObject(sms, "length", length);
                }  // TODO: message parsing
        }

        char type[] = "sms";

        if (success) {
                char msg[] = "Successfully got SMS";
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, data);
        } else { // TODO: +CME ERROR here
                char msg[] = "Failed to get SMS";
                cJSON_Delete(data);
                data = NULL;
                *resp = create_response(success, type, sizeof(type) - 1, msg, sizeof(msg) - 1, NULL);
        }
        return EXIT_SUCCESS;
}

static int quectel_eg06_print_sms(struct cJSON *parser_output)
{
        if (!is_valid_response(parser_output)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = get_response_data(parser_output);
        struct cJSON *messages = cJSON_GetObjectItemCaseSensitive(data, "messages");
        if (!cJSON_IsArray(messages)) {
                return EXIT_FAILURE;
        }
        struct cJSON *item = NULL;
        printf("Messages:\n");
        cJSON_ArrayForEach(item, messages)
        {
                if (!cJSON_IsObject(item)) {
                        continue;
                }
                struct cJSON *index = cJSON_GetObjectItemCaseSensitive(item, "index");
                struct cJSON *stat = cJSON_GetObjectItemCaseSensitive(item, "stat");
                struct cJSON *length = cJSON_GetObjectItemCaseSensitive(item, "length");
                if (!cJSON_IsNumber(index) || !cJSON_IsString(stat) || !cJSON_IsNumber(length)) {
                        continue;
                }
                printf("  - index=%.00f state=%s length=%.00f\n", cJSON_GetNumberValue(index), cJSON_GetStringValue(stat), cJSON_GetNumberValue(length));
        }
        return EXIT_SUCCESS;
}

struct modem quectel_eg06 = {
        .vendor_id = 0x2C7C,
        .product_id = 0x0306,
        .actions = {
                [GET_IMEI] = {
                        .parser = quectel_eg06_parse_imei,
                        .at_cmd = "AT+GSN",
                        .print_parser_resp = quectel_eg06_print_imei,
                },
                [GET_MODEL] = {
                        .parser = quectel_eg06_parse_model,
                        .at_cmd = "ATI",
                        .print_parser_resp = quectel_eg06_print_model,
                },
                [GET_OPERATOR] = {
                        .parser = quectel_eg06_parse_operator,
                        .at_cmd = "AT+COPS=3,1;+COPS?",
                        .print_parser_resp = quectel_eg06_print_operator,
                },
                [GET_CONNECTION_STATUS] = {
                        .parser = quectel_eg06_parse_connection_status,
                        .at_cmd = "AT+CREG=1;+CREG?",
                        .print_parser_resp = quectel_eg06_print_connection_status,
                },
                [GET_BAND] = {
                        .parser = quectel_eg06_parse_band,
                        .at_cmd = "AT+QCFG=\"band\"",
                        .print_parser_resp = quectel_eg06_print_band,
                },
                [GET_SIM_STATUS] = {
                        .parser = quectel_eg06_parse_sim_status,
                        .at_cmd = "AT+QSIMSTAT?",
                        .print_parser_resp = quectel_eg06_print_sim_status,
                },
                [GET_SIGNAL] = {
                        .parser = quectel_eg06_parse_signal,
                        .at_cmd = "AT+CSQ",
                        .print_parser_resp = quectel_eg06_print_signal,
                },
                [GET_TEMPERATURE] = {
                        .parser = quectel_eg06_parse_temperature,
                        .at_cmd = "AT+QTEMP",
                        .print_parser_resp = quectel_eg06_print_temperature,
                },
                [GET_CURRENT_APN] = {
                        .parser = quectel_eg06_parse_apn,
                        .at_cmd = "AT+CGDCONT?",
                        .print_parser_resp = quectel_eg06_print_apn,
                },
                [GET_SERVING_CELL] = {
                        .parser = quectel_eg06_parse_serving_cell,
                        .at_cmd = "AT+QENG=\"servingcell\"",
                        .print_parser_resp = quectel_eg06_print_serving_cell,
                },
                [GET_NEIGHBOUR_CELL] = {
                        .parser = quectel_eg06_parse_neighbour_cells,
                        .at_cmd = "AT+QENG=\"servingcell\";+QENG=\"neighbourcell\"",
                        .print_parser_resp = quectel_eg06_print_neighbour_cells,
                },
                [GET_SMS] = {
                        .parser = quectel_eg06_parse_sms,
                        .at_cmd = "AT+CMGF=0;+CMGL=1",
                        .print_parser_resp = quectel_eg06_print_sms,
                },
        },
};