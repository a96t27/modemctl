#include <commands.h>
#include <cjson/cJSON.h>
#include <text_lines.h>
#include <serial_io.h>
#include <parser.h>
#include <stdio.h>

struct cJSON *create_response(char *method, int success, char *message, struct cJSON *data)
{
        if (method == NULL || message == NULL) {
                return NULL;
        }
        if (data != NULL && !cJSON_IsObject(data)) {
                return NULL;
        }
        struct cJSON *response = cJSON_CreateObject();
        if (response == NULL) {
                return NULL;
        }
        cJSON_AddBoolToObject(response, "success", success);
        cJSON_AddStringToObject(response, "method", method);
        cJSON_AddStringToObject(response, "message", message);
        if (data != NULL) {
                cJSON_AddItemToObject(response, "data", data);
        }
        return response;
}

struct cJSON *get_imei(int fd)
{
        if (fd < 0) {
                return NULL;
        }
        char cmd[] = "AT+GSN";
        struct text_line *lines = at_execute(fd, sizeof(cmd), cmd);
        if (lines == NULL) {
                return create_response("imei", 0, "Failed to execute AT command", NULL);
        }
        struct text_line *temp = lines;
        while (temp != NULL) {
                printf("> %s\n", temp->line);
                temp = temp->next;
        }

        struct cJSON *data = parse_get_imei(lines);
        if (data == NULL) {
                return create_response("imei", 0, "Failed to parse AT command", NULL);
        }
        free_text_lines(&lines);
        return create_response("imei", 1, "Got IMEI code", data);
}