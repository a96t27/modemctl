#include <printing_utils.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <response.h>

int print_at_resp(struct cJSON *at_resp)
{
        if (!is_at_response(at_resp)) {
                return EXIT_FAILURE;
        }
        struct cJSON *data = cJSON_GetObjectItemCaseSensitive(at_resp, "data");
        struct cJSON *item = cJSON_GetObjectItemCaseSensitive(data, "command");
        char *txt = cJSON_GetStringValue(item);
        printf("\n< %s\n", txt);
        item = cJSON_GetObjectItemCaseSensitive(data, "result");
        struct cJSON *line = NULL;
        cJSON_ArrayForEach(line, item)
        {
                txt = cJSON_GetStringValue(line);
                printf("> %s\n", txt);
        }
        return EXIT_SUCCESS;
}

int print_responses(struct cJSON *responses)
{
        if (!cJSON_IsArray(responses)) {
                return EXIT_FAILURE;
        }
        struct cJSON *resp = NULL;
        cJSON_ArrayForEach(resp, responses)
        {
                if (!is_valid_response(resp)) {
                        continue;
                }
                if (is_at_response(resp)) {
                        print_at_resp(resp);
                }
                struct cJSON *msg_json = cJSON_GetObjectItemCaseSensitive(resp, "message");
                char *msg = cJSON_GetStringValue(msg_json);
                printf("%s\n", msg);

        }
        return EXIT_SUCCESS;
}
