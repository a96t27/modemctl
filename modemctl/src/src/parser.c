#include <parser.h>

#include <string.h>
#include <stdbool.h>
#include <stddef.h>


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



// struct cJSON *parse_get_imei(struct text_line *text_lines)
// {
//         if (text_lines == NULL) {
//                 return NULL;
//         }
//         struct cJSON *data = cJSON_CreateObject();
//         if (data == NULL) {
//                 return NULL;
//         }
//         struct text_line *temp = text_lines;
//         skip_empty(temp);
//         while (temp != NULL && strncmp(temp->line, "AT+GSN", temp->len) != 0) {
//                 temp = next_non_empty(temp);
//         }
//         temp = next_non_empty(temp);
//         if (temp == NULL) {
//                 return NULL;
//         }
//         if (is_error(temp)) {
//                 return NULL;
//         }
//         char *imei = temp->line; // TODO: Validate imei code
//         cJSON_AddItemToObject(data, "imei", cJSON_CreateString(imei));
//         return data;
// }
