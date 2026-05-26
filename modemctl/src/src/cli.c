#include <cli.h>
#include <argp.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <string.h>
#include <response.h>

static error_t parse_opt(int key, char *arg, struct argp_state *state);

static char doc[] = "Modemctl - command line tool to control modems";
static char args_doc[] = "";
static struct argp_option options[] = {
        { "imei", 'i', 0, 0, "Get IMEI", 0 },
        { "device", 'd', "PATH_TO_DEVICE", 0, "Set AT command port", 0 },
        { "debug", 'g', 0, 0, "Turn on debug mode", 0 },
        { "json", 'j', 0, 0, "Turn on json output", 0},
        { "at", 'a', "AT_COMMAND", 0, "Execute AT command", 0},
        // { "status", 'u', 0, 0, "Get modem status", 0 },
        // { "all", 'a', 0, 0, "get all", 0},
        // { "operator", 'o', 0 ,0, "Get operator", 0 },
        // { "signal", 's', 0, 0, "Get signal level", 0 },
        // { "cell", 'c', 0, 0, "Get cell", 0 },
        // { "band", 'b', 0, 0, "Get band", 0 },
        // { "watch", 'w', 0, 0, "keep watching", 0 },
        { 0 },
};
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };



static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        struct arguments *arguments = state->input;
        switch (key) {
        case 'd':
                strncpy(arguments->device, arg, PATH_MAX);
                arguments->device[PATH_MAX - 1] = '\0';
                break;
        case 'i':
                arguments->imei = true;
                break;
        case 'u':
                arguments->status = true;
                break;
        case 'g':
                arguments->debug = true;
                break;
        case 'j':
                arguments->json = true;
                break;
        case 'a':
                if (arguments->at_cmds == NULL) {
                        arguments->at_cmds = cJSON_CreateArray();
                }
                struct cJSON *string_json = cJSON_CreateString(arg);
                cJSON_AddItemToArray(arguments->at_cmds, string_json);
                break;
        case ARGP_KEY_ARG:
                if (state->arg_num > 0)
                        argp_usage(state);
                break;
        case ARGP_KEY_END:
                if (state->arg_num > 0)
                        argp_usage(state);
                break;
        default:
                return ARGP_ERR_UNKNOWN;
                break;
        }
        return 0;
}

int parse_arguments(int argc, char **argv, struct arguments *args)
{
        if (argv == NULL || args == NULL) {
                return EXIT_FAILURE;
        }
        return argp_parse(&argp, argc, argv, ARGP_NO_EXIT, 0, args);
}

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
