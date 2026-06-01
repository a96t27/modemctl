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
        struct cJSON *data = cJSON_CreateObject();
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
        char *vendor = NULL;
        size_t vendor_len = 0;
        char *model = NULL;
        size_t model_len = 0;
        char *revision = NULL;
        size_t revision_len = 0;
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
                        vendor = line;
                        vendor_len = len;
                } else if (model == NULL) {
                        model = line;
                        model_len = len;
                } else if (revision == NULL) {
                        revision = line;
                        revision_len = len;
                }
        }
        if (vendor == NULL || model == NULL || revision == NULL) {
                success = false;
        }
        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "model";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Model: %.*s %.*s %.*s", vendor_len, vendor, model_len, model, revision_len, revision);
                cJSON_AddStringToObject(data, "vendor", vendor);
                cJSON_AddStringToObject(data, "model", model);
                cJSON_AddStringToObject(data, "revision", revision);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to read model");
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

static int quectel_eg06_parse_operator(struct cJSON *at_resp, struct cJSON **resp)
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
        char operator_id[17] = { 0 }; // TODO: constant
        size_t operator_id_len = 0;
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
                if (operator_id_len == 0) {
                        sscanf(line, "+COPS: %*d,%*d,\"%16[^\"]\"", operator_id); // TODO: constant
                        operator_id_len = strlen(operator_id);
                }
                if (is_empty(line, strlen(line))
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
        }
        if (operator_id_len == 0) {
                success = false;
        }
        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "model";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Operator: %.*s", operator_id_len, operator_id);
                cJSON_AddStringToObject(data, "operator", operator_id);
        } else { // TODO: +CME ERROR here
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get current operator");
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

static int quectel_eg06_parse_connection_status(struct cJSON *at_resp, struct cJSON **resp)
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
                if (is_empty(line, strlen(line))
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
        }
        char connection_status_buf[20];
        switch (connection_status) {
        case 0:
                snprintf(connection_status_buf, sizeof(connection_status_buf), "%s", "Disconnected");
                break;
        case 1:
                snprintf(connection_status_buf, sizeof(connection_status_buf), "%s", "Connected");
                break;
        case 2:
                snprintf(connection_status_buf, sizeof(connection_status_buf), "%s", "Connecting");
                break;
        case 3:
                snprintf(connection_status_buf, sizeof(connection_status_buf), "%s", "Registration denied");
                break;
        case 5:
                snprintf(connection_status_buf, sizeof(connection_status_buf), "%s", "Connected (roaming)");
                break;
        default:
                snprintf(connection_status_buf, sizeof(connection_status_buf), "%s", "Unknown");
                break;
        }
        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "model";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Connection: %s", connection_status_buf);
                cJSON_AddStringToObject(data, "connection_text", connection_status_buf);
                cJSON_AddNumberToObject(data, "connection_code", (double)connection_status);
        } else { // TODO: +CME ERROR here
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get connection status");
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
        struct cJSON *at_command = cJSON_GetObjectItemCaseSensitive(at_data, "command");
        char *cmd = cJSON_GetStringValue(at_command);
        size_t cmd_len = strlen(cmd);
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
                if (is_empty(line, strlen(line))
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
        }

        struct cJSON *bands_json = cJSON_CreateArray();
        cJSON_AddItemToObject(data, "bands", bands_json);

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        char msg_begining[] = "Bands: ";
        char msg_len = sizeof(msg_begining) - 1;
        strncat(msg_buf, msg_begining, sizeof(msg_buf));
        char sep[] = ", ";
        bool first = true;

        for (int i = 0;i < wcdma_bands_len; i++) {
                struct band *b = &wcdma_bands[i];
                if (!(b->mask & bandval)) {
                        continue;
                }

                if (!first) {
                        strncat(msg_buf + msg_len, sep, sizeof(msg_buf) - msg_len);
                        msg_len += sizeof(sep) - 1;
                }
                first = false;
                size_t name_len = strlen(b->name);
                strncat(msg_buf + msg_len, b->name, sizeof(msg_buf) - msg_len);
                msg_len += name_len;
                cJSON_AddItemToArray(bands_json, cJSON_CreateString(b->name));
        }

        for (int i = 0;i < lte_bands_len; i++) {
                struct band *b = &lte_bands[i];
                if (!(b->mask & ltebandval)) {
                        continue;
                }

                if (!first) {
                        strncat(msg_buf + msg_len, sep, sizeof(msg_buf) - msg_len);
                        msg_len += sizeof(sep) - 1;
                }
                first = false;
                size_t name_len = strlen(b->name);
                strncat(msg_buf + msg_len, b->name, sizeof(msg_buf) - msg_len);
                msg_len += name_len;
                cJSON_AddItemToArray(bands_json, cJSON_CreateString(b->name));
        }
        const char *type = NULL;

        if (success) {
                type = "model";
        } else { // TODO: +CME ERROR here
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get band");
                type = "error";
                cJSON_Delete(data);
                data = NULL;
        }
        *resp = create_response(success, type, strlen(type), msg_buf, msg_len, data);
        return EXIT_SUCCESS;
}

static int quectel_eg06_parse_sim_status(struct cJSON *at_resp, struct cJSON **resp)
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
                sscanf(line, "+QSIMSTAT: %*d,%d", &sim_state); // TODO: constant

                if (is_empty(line, strlen(line))
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
        }
        char state_str[10];
        switch (sim_state) {
        case 0:
                strncpy(state_str, "removed", sizeof(state_str) - 1);
                break;
        case 1:
                strncpy(state_str, "inserted", sizeof(state_str) - 1);
                break;
        default:
                strncpy(state_str, "unknown", sizeof(state_str) - 1);
                break;
        }

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };

        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "model";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "SIM state: %.*s", sizeof(state_str), state_str);
                cJSON_AddStringToObject(data, "state", state_str);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get current operator");
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

static int quectel_eg06_parse_signal(struct cJSON *at_resp, struct cJSON **resp)
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

                if (is_empty(line, strlen(line))
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
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

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };

        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "model";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Signal strength: %.*s", sizeof(strength_str), strength_str);
                cJSON_AddStringToObject(data, "signal_strength", strength_str);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get signal strength");
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

static int quectel_eg06_parse_temperature(struct cJSON *at_resp, struct cJSON **resp)
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

                if (is_empty(line, strlen(line))
                        || strncmp(line, cmd, cmd_len) == 0
                        || is_unsolicited_event(line, len)) {
                        continue;
                }
        }
        double avg_temp = (t1 + t2 + t3) / 3.0;

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };

        int msg_len = 0;
        const char *type = NULL;

        if (success) {
                type = "model";
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Temperature: %.1fC", avg_temp);
                cJSON_AddNumberToObject(data, "temperature", avg_temp);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get temperature");
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

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        char msg_len = 0;
        bool first = true;
        char beginning[] = "APNs: ";
        char sep[] = ", ";
        strncat(msg_buf, beginning, sizeof(msg_buf));
        msg_len += sizeof(beginning) - 1;

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
                if (sscanf(line, "+CGDCONT: %*d,\"%*[^\"]\",\"%64[^\"]\"", apn) == 1) {
                        cJSON_AddItemToArray(apns, cJSON_CreateString(apn));
                        if (!first) {
                                strncat(msg_buf + msg_len, sep, sizeof(msg_buf) - msg_len);
                                msg_len += sizeof(sep) - 1;
                        }
                        first = false;
                        strncat(msg_buf + msg_len, apn, sizeof(msg_buf) - msg_len);
                        msg_len += strlen(apn);
                }
        }

        const char *type = NULL;

        if (success) {
                type = "model";
        } else { // TODO: +CME ERROR here
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get APN");
                type = "error";
                cJSON_Delete(data);
                data = NULL;
        }
        *resp = create_response(success, type, strlen(type), msg_buf, msg_len, data);
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
        bool success = true;

        char state[16] = { 0 };
        char generation[16] = { 0 };
        uint64_t cell_id = 0;

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
                sscanf(line, "+QENG: \"servingcell\",\"%12[^\"]\",\"%12[^\"]\",%*[^,],%*[^,],%*[^,],%llx", state, generation, &cell_id);
        }

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        char msg_len = 0;
        msg_len = snprintf(msg_buf, sizeof(msg_buf), "Serving Cell: %s %s (cell_id:%llx)", generation, state, cell_id);
        const char *type = NULL;

        if (success) {
                type = "model";
                cJSON_AddStringToObject(data, "generation", generation);
                cJSON_AddStringToObject(data, "state", state);
                cJSON_AddNumberToObject(data, "cell_id", cell_id);
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get band");
                type = "error";
                cJSON_Delete(data);
                data = NULL;
        }
        *resp = create_response(success, type, strlen(type), msg_buf, msg_len, data);
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

        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        size_t msg_len = 0;

        char beginning[] = "Neighbour cells:\n";
        strncpy(msg_buf, beginning, sizeof(msg_buf));
        msg_len += sizeof(beginning) - 1;

        enum qeng_mode mode = QENG_UNKNOWN;
        char mode_buf[8] = { 0 };
        const char lte[] = "LTE";
        const char wcdma[] = "WCDMA";

        char cell_type[8];
        char id[16];

        struct cJSON *cells = cJSON_CreateArray();
        cJSON_AddItemToObject(data, "cells", cells);
        struct cJSON *cell = NULL;

        char buf[128] = { 0 };
        size_t buf_len = 0;

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
                        if (strncmp(mode_buf, lte, sizeof(lte) - 1) == 0) {
                                mode = QENG_LTE;
                        } else if (strncmp(mode_buf, wcdma, sizeof(wcdma) - 1) == 0) {
                                mode = QENG_WCDMA;
                        }
                } else if (sscanf(line, "+QENG: \"neighbourcell%*[^\"]\",\"%7[^\"]\",%*[^,],%15[^,],", cell_type, id) == 2) {
                        cell = cJSON_CreateObject();
                        cJSON_AddItemToArray(cells, cell);
                        cJSON_AddStringToObject(cell, "cell_type", cell_type);
                        cJSON_AddStringToObject(cell, "id", id);
                        buf_len = snprintf(buf, sizeof(buf), "\t- type: %s, id: %s\n", cell_type, id);
                        strncat(msg_buf + msg_len, buf, sizeof(msg_buf) - msg_len);
                        msg_len += buf_len;

                }
        }
        msg_buf[msg_len - 1] = '\0';
        const char *type = NULL;

        if (success) {
                type = "neighbourcell";
        } else {
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "Failed to get serving cell");
                type = "error";
                cJSON_Delete(data);
                data = NULL;
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
                [GET_MODEL] = {
                        .parser = quectel_eg06_parse_model,
                        .at_cmd = "ATI",
                },
                [GET_OPERATOR] = {
                        .parser = quectel_eg06_parse_operator,
                        .at_cmd = "AT+COPS=3,1;+COPS?",
                },
                [GET_CONNECTION_STATUS] = {
                        .parser = quectel_eg06_parse_connection_status,
                        .at_cmd = "AT+CREG=1;+CREG?",
                },
                [GET_BAND] = {
                        .parser = quectel_eg06_parse_band,
                        .at_cmd = "AT+QCFG=\"band\"",
                },
                [GET_SIM_STATUS] = {
                        .parser = quectel_eg06_parse_sim_status,
                        .at_cmd = "AT+QSIMSTAT?",
                },
                [GET_SIGNAL] = {
                        .parser = quectel_eg06_parse_signal,
                        .at_cmd = "AT+CSQ",
                },
                [GET_TEMPERATURE] = {
                        .parser = quectel_eg06_parse_temperature,
                        .at_cmd = "AT+QTEMP",
                },
                [GET_CURRENT_APN] = {
                        .parser = quectel_eg06_parse_apn,
                        .at_cmd = "AT+CGDCONT?"
                },
                [GET_SERVING_CELL] = {
                        .parser = quectel_eg06_parse_serving_cell,
                        .at_cmd = "AT+QENG=\"servingcell\"",
                },
                [GET_NEIGHBOUR_CELL] = {
                        .parser = quectel_eg06_parse_neighbour_cells,
                        .at_cmd = "AT+QENG=\"servingcell\";+QENG=\"neighbourcell\"",
                },
        },
};