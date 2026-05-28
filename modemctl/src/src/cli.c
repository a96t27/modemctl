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
        { "device", DEVICE_KEY, "PATH_TO_DEVICE", 0, "Set AT command port", 0 },
        { "debug", DEBUG_KEY, 0, 0, "Turn on debug mode", 0 },
        { "json", JSON_KEY, 0, 0, "Turn on json output", 0},
        { "at", AT_KEY, "AT_COMMAND", 0, "Execute AT command", 0},
        { "all", ALL_KEY, 0, 0, "get all", 0},
        { "watch", WATCH_KEY, 0, 0, "keep watching", 0 },
        { "imei", IMEI_KEY, 0, 0, "Get IMEI", 0 },
        { "model", MODEL_KEY, 0, 0, "Get modem model", 0 },
        { "operator", OPERATOR_KEY, 0 ,0, "Get operator", 0 },
        { "connection", CONNECTION_KEY, 0, 0, "Get connection status", 0 },
        { "band", BAND_KEY, 0, 0, "Get current band", 0 },
        { "sim", SIM_KEY, 0, 0, "Get sim status", 0 },
        { "serving-cell", SERVING_CELL_KEY, 0, 0, "Get current serving cell", 0 },
        { "neighbour-cells", NEIGHBOUR_CELLS_KEY, 0, 0, "Get neighbour cells", 0 },
        { "signal", SIGNAL_KEY, 0, 0, "Get signal level", 0 },
        { "ip", IP_KEY, 0, 0, "Get ip address", 0 },
        { "apn", APN_KEY, 0, 0, "Get current APN", 0 },
        { "number", NUMBER_KEY, 0, 0, "Get phone number", 0 },
        { "sms", SMS_KEY, 0, 0, "Get SMS", 0 },
        { "temperature", TEMPERATURE_KEY, 0, 0, "Get temperature", 0 },
        { 0 },
};
static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        struct arguments *arguments = state->input;
        switch (key) {
        case DEVICE_KEY:
                strncpy(arguments->device, arg, PATH_MAX);
                arguments->device[PATH_MAX - 1] = '\0';
                break;
        case DEBUG_KEY:
                arguments->debug = true;
                break;
        case JSON_KEY:
                arguments->json = true;
                break;
        case AT_KEY:
                if (arguments->at_cmds == NULL) {
                        arguments->at_cmds = cJSON_CreateArray();
                }
                struct cJSON *string_json = cJSON_CreateString(arg);
                cJSON_AddItemToArray(arguments->at_cmds, string_json);
                break;
        case WATCH_KEY:
                return ARGP_ERR_UNKNOWN; //TODO make --watch option
        case ALL_KEY:
                for (int i = 0; i < __ACTIONS_MAX; i++) {
                        arguments->actions[i] = true;
                }
                break;
        case IMEI_KEY:
                arguments->actions[GET_IMEI] = true;
                break;
        case MODEL_KEY:
                arguments->actions[GET_MODEL] = true;
                break;
        case OPERATOR_KEY:
                arguments->actions[GET_OPERATOR] = true;
                break;
        case CONNECTION_KEY:
                arguments->actions[GET_CONNECTION_STATUS] = true;
                break;
        case BAND_KEY:
                arguments->actions[GET_BAND] = true;
                break;
        case SIM_KEY:
                arguments->actions[GET_SIM_STATUS] = true;
                break;
        case SERVING_CELL_KEY:
                arguments->actions[GET_SERVING_CELL] = true;
                break;
        case NEIGHBOUR_CELLS_KEY:
                arguments->actions[GET_NEIGHBOUR_CELL] = true;
                break;
        case SIGNAL_KEY:
                arguments->actions[GET_SIGNAL] = true;
                break;
        case IP_KEY:
                arguments->actions[GET_IP_ADDRESS] = true;
                break;
        case APN_KEY:
                arguments->actions[GET_CURRENT_APN] = true;
                break;
        case NUMBER_KEY:
                arguments->actions[GET_PHONE_NUMBER] = true;
                break;
        case SMS_KEY:
                arguments->actions[GET_SMS] = true;
                break;
        case TEMPERATURE_KEY:
                arguments->actions[GET_TEMPERATURE] = true;
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
        printf("\n< %s\n\n", txt);
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
