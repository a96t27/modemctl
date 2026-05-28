#include <response.h>

#include <stddef.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


struct cJSON *create_response(bool success, const char *type, size_t type_len, const char *message, size_t message_len, struct cJSON *data)
{
        if (type == NULL || message == NULL || type_len == 0 || message_len == 0) {
                return NULL;
        }
        struct cJSON *resp = cJSON_CreateObject();
        if (resp == NULL) {
                return NULL;
        }

        cJSON_AddBoolToObject(resp, "success", success);

        char buf[RESPONSE_BUFFER_SIZE] = { 0 };

        snprintf(buf, sizeof(buf), "%.*s", type_len, type);
        cJSON_AddStringToObject(resp, "type", buf);

        snprintf(buf, sizeof(buf), "%.*s", message_len, message);
        cJSON_AddStringToObject(resp, "message", buf);

        if (cJSON_IsObject(data)) {
                cJSON_AddItemToObject(resp, "data", data);
        }
        return resp;
}

bool is_valid_response(struct cJSON *resp)
{
        if (!cJSON_IsObject(resp)) {
                return false;
        }
        struct cJSON *item = cJSON_GetObjectItemCaseSensitive(resp, "success");
        if (!cJSON_IsBool(item)) {
                return false;
        }
        item = cJSON_GetObjectItemCaseSensitive(resp, "type");
        if (!cJSON_IsString(item)) {
                return false;
        }
        item = cJSON_GetObjectItemCaseSensitive(resp, "message");
        if (!cJSON_IsString(item)) {
                return false;
        }
        item = cJSON_GetObjectItemCaseSensitive(resp, "data");
        if (item != NULL && !cJSON_IsObject(item)) {
                return false;
        }
        return true;
}

bool is_at_response(struct cJSON *resp)
{
        if (!is_valid_response(resp)) {
                return false;
        }
        char *type = get_response_type(resp);
        struct cJSON *at_data = get_response_data(resp);

        if (strcmp(type, "at") != 0) {
                return false;
        }
        if (at_data == NULL) {
                return false;
        }
        struct cJSON *at_result = cJSON_GetObjectItemCaseSensitive(at_data, "result");
        if (!cJSON_IsArray(at_result)) {
                return false;
        }
        struct cJSON *at_command = cJSON_GetObjectItemCaseSensitive(at_data, "command");
        if (!cJSON_IsString(at_command)) {
                return false;
        }
        return true;
}

char *get_response_type(struct cJSON *resp)
{
        if (!cJSON_IsObject(resp)) {
                return NULL;
        }
        struct cJSON *item = cJSON_GetObjectItemCaseSensitive(resp, "type");
        if (!cJSON_IsString(item)) {
                return NULL;
        }
        return cJSON_GetStringValue(item);
}

char *get_response_message(struct cJSON *resp)
{
        if (!cJSON_IsObject(resp)) {
                return NULL;
        }
        struct cJSON *item = cJSON_GetObjectItemCaseSensitive(resp, "message");
        if (!cJSON_IsString(item)) {
                return NULL;
        }
        return cJSON_GetStringValue(item);
}

bool get_response_success(struct cJSON *resp)
{
        if (!cJSON_IsObject(resp)) {
                return false;
        }
        struct cJSON *item = cJSON_GetObjectItemCaseSensitive(resp, "success");
        if (!cJSON_IsBool(item)) {
                return false;
        }
        return cJSON_IsTrue(item);
}

struct cJSON *get_response_data(struct cJSON *resp)
{
        if (!cJSON_IsObject(resp)) {
                return false;
        }
        struct cJSON *item = cJSON_GetObjectItemCaseSensitive(resp, "data");
        if (!cJSON_IsObject(item)) {
                return NULL;
        }
        return item;
}

struct cJSON *create_execution_error_response(const char *message, size_t message_len)
{
        char type[] = "execution";
        return create_response(false, type, sizeof(type) - 1, message, message_len, NULL);
}