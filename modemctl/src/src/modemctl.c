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
#include <printing_utils.h>

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
        { "device", 'd', "PATH_TO_DEVICE", 0, "Set AT command port", 0 },
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

int execute_cmds(struct ModemctlContext *ctx, struct cJSON *cmds, struct cJSON *responses)
{
        if (ctx == NULL || ctx->fd < 0 || !cJSON_IsArray(cmds) || !cJSON_IsArray(responses)) {
                return EXIT_FAILURE;
        }
        struct cJSON *cmd = NULL;
        cJSON_ArrayForEach(cmd, cmds)
        {
                if (!cJSON_IsString(cmd)) {
                        continue;
                }
                char *txt = cJSON_GetStringValue(cmd);
                struct cJSON *resp = at_execute(ctx, txt, strlen(txt));
                if (resp == NULL) {
                        continue; // TODO: reikia generuoti klaida
                }
                cJSON_AddItemToArray(responses, resp);
        }
        return EXIT_SUCCESS;
}

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
        int ret = EXIT_SUCCESS;
        if (argp_parse(&_argp, argc, argv, ARGP_NO_EXIT, 0, &args) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
                goto err_failed_argp;
        }

        int fd = -1;
        if (strnlen(args.device, PATH_MAX) > 0) {
                fd = at_get_port(args.device);
        } else {
                fd = at_find_port();
        }
        if (fd < 0) {
                printf("Failed to open device\n");
                ret = EXIT_FAILURE;
                goto err_failed_to_open_file;
        }

        at_setup_port(fd);
        struct ModemctlContext ctx = {
                .fd = fd,
        };

        struct cJSON *responses = cJSON_CreateArray();

        if (args.at_cmds != NULL) {
                if (execute_cmds(&ctx, args.at_cmds, responses) != EXIT_SUCCESS) {
                        printf("Failed to execute commands\n");
                }
        }

        if (args.imei) { // TODO: ideti i atskira funkcija?
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
                print_responses(responses);
        }
        cJSON_Delete(responses);
        close(fd);
err_failed_to_open_file:
err_failed_argp:
        if (args.at_cmds) {
                cJSON_Delete(args.at_cmds);
        }
        return ret;
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

