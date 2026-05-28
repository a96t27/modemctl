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

int running = 1;

void sig_handler(int sigint)
{
        (void)sigint;
        running = 0;
}

int execute_cmds(struct at_port *port, struct cJSON *cmds, struct cJSON *responses)
{
        if (port == NULL || !cJSON_IsArray(cmds) || !cJSON_IsArray(responses)) {
                return EXIT_FAILURE;
        }
        struct cJSON *cmd = NULL;
        cJSON_ArrayForEach(cmd, cmds)
        {
                if (!cJSON_IsString(cmd)) {
                        continue;
                }
                char *txt = cJSON_GetStringValue(cmd);
                struct cJSON *resp = at_execute(port, txt, strlen(txt));
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

        struct arguments args = { 0 };
        int ret = EXIT_SUCCESS;
        if (parse_arguments(argc, argv, &args) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
                goto err_failed_argp;
        }

        int device_len = strnlen(args.device, PATH_MAX);
        struct at_port at_port = { 0 };

        struct cJSON *responses = cJSON_CreateArray();

        if (device_len > 0) {
                if (get_at_port(args.device, device_len, &at_port) != EXIT_SUCCESS) {
                        char err[] = "Unsupported device";
                        cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                        ret = EXIT_FAILURE;
                        goto err_failed_to_open_file;
                }
        } else {
                if (find_any_at_port(&at_port) != EXIT_SUCCESS) {
                        char err[] = "Failed to find an AT port";
                        cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                        ret = EXIT_FAILURE;
                        goto err_failed_to_open_file;
                }
        }

        if (setup_at_port(&at_port) != EXIT_SUCCESS) {
                char err[] = "Failed to setup the AT port";
                cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                ret = EXIT_FAILURE;
                goto err_failed_to_open_file;
        }



        if (args.at_cmds != NULL) {
                if (execute_cmds(&at_port, args.at_cmds, responses) != EXIT_SUCCESS) {
                        char err[] = "Failed to execute AT commands";
                        cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                }
        }

        struct modem modem = { 0 };
        if (get_modem(at_port.usb_info.vendor_id, at_port.usb_info.product_id, &modem) != EXIT_SUCCESS) {
                char err[] = "Failed to find modem";
                cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                goto err_failed_to_open_file;
        }

        for (int i = 0; i < __ACTIONS_MAX; i++) {
                if (!args.actions[i]) {
                        continue;
                }
                struct action *action = &modem.actions[i];
                if (action->at_cmd == NULL || action->parser == NULL || strlen(action->at_cmd) == 0) {
                        char err[] = "Unsupported action";
                        cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                        continue;
                }
                struct cJSON *at_resp = at_execute(&at_port, action->at_cmd, strlen(action->at_cmd));
                if (at_resp == NULL) {
                        char err[] = "Failed to execute an action";
                        cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                        continue;
                }

                if (args.debug) {
                        cJSON_AddItemToArray(responses, at_resp);
                }

                struct cJSON *parser_resp = NULL;
                if (action->parser(at_resp, &parser_resp) == EXIT_SUCCESS) {
                        cJSON_AddItemToArray(responses, parser_resp);
                } else {
                        char err[] = "Failed to AT response";
                        cJSON_AddItemToArray(responses, create_execution_error_response(err, sizeof(err) - 1));
                        cJSON_Delete(parser_resp);
                }

                if (!args.debug) {
                        cJSON_Delete(at_resp);
                }
        }

err_failed_to_open_file:
        if (args.json) {
                char *txt = cJSON_Print(responses);
                printf("%s\n", txt);
                free(txt);
        } else {
                print_responses(responses);
        }
        cJSON_Delete(responses);
        close(at_port.fd);
err_failed_argp:
        if (args.at_cmds) {
                cJSON_Delete(args.at_cmds);
        }
        return ret;
}
