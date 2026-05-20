#include <parser.h>

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <response.h>
#include <stdio.h>

bool is_empty(const char *line, size_t len)
{
        return line == NULL || len == 0;
}


bool is_ok(const char *line, size_t len)
{
        if (is_empty(line, len)) {
                return false;
        }
        char ok[] = "OK";
        return (len == sizeof(ok) - 1) && (strncmp(line, ok, sizeof(ok) - 1) == 0);
}


bool is_error(const char *line, size_t len)
{
        if (is_empty(line, len)) {
                return false;
        }
        char error[] = "ERROR";
        if ((len == sizeof(error) - 1) && (strncmp(line, error, sizeof(error) - 1) == 0)) {
                return true;
        }
        char cme_error[] = "+CME ERROR";
        if ((len >= sizeof(cme_error) - 1) && (strncmp(line, cme_error, sizeof(cme_error) - 1) == 0)) {
                return true;
        }
        char cms_error[] = "+CMS ERROR";
        if ((len >= sizeof(cms_error) - 1) && (strncmp(line, cms_error, sizeof(cms_error) - 1) == 0)) {
                return true;
        }
        return false;
}


bool is_end(const char *line, size_t len)
{
        return is_ok(line, len) || is_error(line, len);
}

bool is_unsolicited_event(const char *line, size_t len) // gal but reikia filtruoti konkrecius pranesimus; turi buti tikrinama po klaidos tikrinimo
{
        if (is_empty(line, len)) {
                return false;
        }
        return line[0] == '+';
}

struct cJSON *parse_get_imei(struct cJSON *at_resp)
{
        if (!is_at_response(at_resp)) {
                return NULL;
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
                msg_len = snprintf(msg_buf, sizeof(msg_buf), "IMEI: %.*s", imei_len, imei);
                type = "imei";
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
        return create_response(success, type, strlen(type), msg_buf, msg_len, data);
}
