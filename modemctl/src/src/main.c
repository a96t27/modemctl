#include <stdio.h>
#include <argp.h>
#include <signal.h>

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
#include <transport.h>
#include <response.h>
#include <parser.h>
#include <cli.h>
#include <modem.h>
#include <modem_lookup.h>
#include <context.h>

bool running = true;

void sig_handler(int sigint)
{
        (void)sigint;
        running = false;
}

int execute_cmds(struct context *ctx, struct cJSON *cmds)
{
        if (!is_valid_context(ctx) || !cJSON_IsArray(cmds)) {
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
                        continue;
                }
                if (ctx->json) {
                        char *txt = cJSON_Print(resp);
                        printf("%s\n", txt);
                        free(txt);
                } else {
                        print_at_resp(resp);
                }
                cJSON_Delete(resp);
        }
        return EXIT_SUCCESS;
}

int execute_action(struct context *ctx, enum action_code action_code)
{
        if (!is_valid_context(ctx) || action_code < 0 || action_code >= __ACTIONS_MAX) {
                return EXIT_FAILURE;
        }
        struct action *action = &ctx->modem->actions[action_code];

        if (!is_action_implemented(action)) {
                return EXIT_FAILURE;
        }

        int ret = EXIT_SUCCESS;
        struct cJSON *at_response = at_execute(ctx, action->at_cmd, strlen(action->at_cmd));
        if (at_response == NULL || !get_response_success(at_response)) {
                ret = EXIT_FAILURE;
                goto err_no_parsing;
        }

        if (ctx->debug) {
                if (ctx->json) {
                        char *txt = cJSON_Print(at_response);
                        printf("%s\n", txt);
                        free(txt);
                } else {
                        print_at_resp(at_response);
                }
        }

        struct cJSON *parser_resp = NULL;
        action->parser(at_response, &parser_resp);
        if (ctx->json) {
                char *txt = cJSON_Print(parser_resp);
                printf("%s\n", txt);
                free(txt);
        } else if (action->print_parser_resp(parser_resp) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
        }
        cJSON_Delete(parser_resp);
err_no_parsing:
        cJSON_Delete(at_response);

        return ret;
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

        struct arguments args = { 0 };
        int ret = EXIT_SUCCESS;
        if (parse_arguments(argc, argv, &args) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
                goto err_failed_argp;
        }

        int device_len = strnlen(args.device, PATH_MAX);
        struct at_port at_port = { 0 };

        if (device_len > 0) {
                if (get_at_port(args.device, device_len, &at_port) != EXIT_SUCCESS) {
                        printf("Unsupported device\n");
                        ret = EXIT_FAILURE;
                        goto err_failed_to_open_file;
                }
        } else {
                if (find_any_at_port(&at_port) != EXIT_SUCCESS) {
                        printf("Failed to find an AT port\n");
                        ret = EXIT_FAILURE;
                        goto err_failed_to_open_file;
                }
        }

        if (setup_at_port(&at_port) != EXIT_SUCCESS) {
                printf("Failed to setup the AT port\n");
                ret = EXIT_FAILURE;
                goto err_failed_to_open_file;
        }

        struct modem modem = { 0 };
        if (get_modem(at_port.usb_info.vendor_id, at_port.usb_info.product_id, &modem) != EXIT_SUCCESS) {
                printf("Failed to find modem\n");
                ret = EXIT_FAILURE;
                goto err_failed_to_open_file;
        }

        struct context ctx = {
                .debug = args.debug,
                .json = args.json,
                .modem = &modem,
                .port = &at_port,
                .running = &running,
        };

        bool first = true;
        while ((args.watch || first) && running) {
                first = false;
                for (int a = 0; a < __ACTIONS_MAX; a++) {
                        if (!running) {
                                break;
                        }
                        if (!args.actions[a] && !args.all) {
                                continue;
                        }
                        if (args.actions[a] && !is_action_implemented(modem.actions + a)) {
                                if (!args.json) {
                                        printf("Not implemented\n");
                                }
                                continue;
                        }
                        if (args.all && !is_action_implemented(modem.actions + a)) {
                                continue;
                        }
                        for (int try = 1; try <= at_port.retry_max; try++) {
                                if (!running) {
                                        break;
                                }
                                if (execute_action(&ctx, a) == EXIT_SUCCESS) {
                                        break;
                                }
                                if (!args.json) {
                                        printf("Failed to execute action (%d/%d)...\n", try, at_port.retry_max);
                                }
                        }
                }

                if (args.at_cmds != NULL) {
                        if (running && execute_cmds(&ctx, args.at_cmds) != EXIT_SUCCESS) {
                                printf("Failed to execute AT commands\n");
                        }
                }
                if (args.watch) {
                        sleep(ctx.port->timeout_seconds);
                }
        }

err_failed_to_open_file:
        close(at_port.fd);
err_failed_argp:
        if (args.at_cmds) {
                cJSON_Delete(args.at_cmds);
        }
        return ret;
}
