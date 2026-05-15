#include <parser.h>
#include <string.h>


int is_empty(struct text_line *text_line)
{
        return text_line == NULL || text_line->len == 0;
}


int is_ok(struct text_line *text_line)
{
        if (is_empty(text_line)) {
                return 0;
        }
        char ok[] = "OK";
        return strncmp(text_line->line, ok, sizeof(ok)) == 0;
}


int is_error(struct text_line *text_line)
{
        if (is_empty(text_line)) {
                return 0;
        }
        char error[] = "ERROR";
        return strncmp(text_line->line, error, sizeof(error)) == 0;
}


int is_end(struct text_line *text_line)
{
        return is_ok(text_line) || is_error(text_line);
}

struct text_line *skip_empty(struct text_line *text_lines)
{
        if (text_lines == NULL) {
                return NULL;
        }
        struct text_line *temp = text_lines;
        while (temp != NULL && is_empty(temp)) {
                temp = temp->next;
        }
        return temp;
}

struct text_line *next_non_empty(struct text_line *text_lines)
{
        if (text_lines == NULL) {
                return NULL;
        }
        struct text_line *temp = text_lines->next;
        return skip_empty(temp);
}



struct cJSON *parse_get_imei(struct text_line *text_lines)
{
        if (text_lines == NULL) {
                return NULL;
        }
        struct cJSON *data = cJSON_CreateObject();
        if (data == NULL) {
                return NULL;
        }
        struct text_line *temp = text_lines;
        skip_empty(temp);
        while (temp != NULL && strncmp(temp->line, "AT+GSN", temp->len) != 0) {
                temp = next_non_empty(temp);
        }
        temp = next_non_empty(temp);
        if (temp == NULL) {
                return NULL;
        }
        if (is_error(temp)) {
                return NULL;
        }
        char *imei = temp->line; // TODO: Validate imei code
        cJSON_AddItemToObject(data, "imei", cJSON_CreateString(imei));
        return data;
}
