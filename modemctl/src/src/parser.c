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
        return (len == sizeof(error) - 1) && (strncmp(line, error, sizeof(error) - 1) == 0);
}


bool is_end(const char *line, size_t len)
{
        return is_ok(line, len) || is_error(line, len);
}

// struct text_line *skip_empty(struct text_line *text_lines)
// {
//         if (text_lines == NULL) {
//                 return NULL;
//         }
//         struct text_line *temp = text_lines;
//         while (temp != NULL && is_empty(temp)) {
//                 temp = temp->next;
//         }
//         return temp;
// }

// struct text_line *next_non_empty(struct text_line *text_lines)
// {
//         if (text_lines == NULL) {
//                 return NULL;
//         }
//         struct text_line *temp = text_lines->next;
//         return skip_empty(temp);
// }

bool is_unsolicited_event(const char *line, size_t len) // gal but reikia filtruoti konkrecius pranesimus
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
        cJSON_AddStringToObject(data, "imei", imei);
        char msg_buf[RESPONSE_BUFFER_SIZE] = { 0 };
        int msg_len = snprintf(msg_buf, sizeof(msg_buf), "IMEI: %.*s", imei_len, imei); // kartais reiksme neigema
        msg_len = msg_len < sizeof(msg_buf) - 2 ? msg_len : sizeof(msg_buf) - 2;
        char type[] = "imei";
        return create_response(success, type, sizeof(type) - 1, msg_buf, msg_len, data);
}
