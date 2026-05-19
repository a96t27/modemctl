#include <stdio.h>
#include <argp.h>
#include <signal.h>
#include <serial_io.h>

#include <serial_io.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <math.h>
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <modemctl_context.h>
#include <response.h>
#include <parser.h>

#define DEFAULT_DEVICE "/dev/ttyUSB2"

int running = 1;

void sig_handler(int sigint)
{
        (void)sigint;
        running = 0;
}


static error_t parse_opt(int key, char *arg, struct argp_state *state);
struct Arguments {
        bool all;
        bool imei;
        bool opertaor;
        bool signal;
        bool cell;
        bool band;
        bool status;
        bool json;
        char device[PATH_MAX];
        bool debug;
        bool watch;
        struct cJSON *at_cmds; // array of strings
};

static char doc[] = "Modemctl - command line tool to control modems";
static char args_doc[] = "";
static struct argp_option options[] = {
        { "imei", 'i', 0, 0, "Get IMEI", 0 },
        { "device", 'd', DEFAULT_DEVICE, 0, "Set AT command port", 0 },
        { "debug", 'g', 0, 0, "Toggle debug mode", 0 },
        { "json", 'j', 0, 0, "Get json", 0},
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
static struct argp _argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int main(int argc, char **argv)
{
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        signal(SIGQUIT, sig_handler);
        signal(SIGABRT, SIG_IGN);
        signal(SIGILL, SIG_IGN);
        signal(SIGFPE, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGIO, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);
        signal(SIGUSR2, SIG_IGN);

        struct Arguments args = { 0 };
        strcpy(args.device, DEFAULT_DEVICE);

        argp_parse(&_argp, argc, argv, 0, 0, &args);
        int fd = at_get_port(args.device);
        if (fd < 0) {
                printf("Failed to open device\n");
                return EXIT_FAILURE;
        }

        at_setup_port(fd);
        struct ModemctlContext ctx = {
                .fd = fd,
        };
        struct cJSON *responses = cJSON_CreateArray();
        if (args.at_cmds != NULL) {
                struct cJSON *cmd = NULL;
                cJSON_ArrayForEach(cmd, args.at_cmds)
                {
                        char *txt = cJSON_GetStringValue(cmd);
                        struct cJSON *resp = at_execute(&ctx, txt, strlen(txt));
                        cJSON_AddItemToArray(responses, resp);
                }
        }

        if (args.imei) {
                char cmd[] = "AT+GSN";
                struct cJSON *at_resp = at_execute(&ctx, cmd, sizeof(cmd) - 1);
                struct cJSON *parser_resp = parse_get_imei(at_resp);
                if (args.debug) {
                        cJSON_AddItemToArray(responses, at_resp);
                } else {
                        cJSON_Delete(at_resp);
                }
                cJSON_AddItemToArray(responses, parser_resp);
        }

        if (args.json) {
                char *txt = cJSON_Print(responses);
                printf("%s\n", txt);
                free(txt);
        } else {
                struct cJSON *resp = NULL;
                cJSON_ArrayForEach(resp, responses)
                {
                        if (!is_valid_response(resp)) {
                                continue;
                        }
                        struct cJSON *type_json = cJSON_GetObjectItemCaseSensitive(resp, "type");
                        char *type = cJSON_GetStringValue(type_json);
                        struct cJSON *data = cJSON_GetObjectItemCaseSensitive(resp, "data");
                        if (strcmp(type, "at") == 0 && cJSON_IsObject(data)) {
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
                        } else {
                                struct cJSON *msg_json = cJSON_GetObjectItemCaseSensitive(resp, "message");
                                char *msg = cJSON_GetStringValue(msg_json);
                                printf("%s\n", msg);
                        }
                }
        }
        cJSON_Delete(responses);
        close(fd);
        if (args.at_cmds) {
                cJSON_Delete(args.at_cmds);
        }
        return EXIT_SUCCESS;
}


static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        struct Arguments *arguments = state->input;
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

