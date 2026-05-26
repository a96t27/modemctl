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
        if (device_len > 0) {
                if (get_at_port(args.device, device_len, &at_port) != EXIT_SUCCESS) {
                        printf("Unsupported device\n");
                        ret = EXIT_FAILURE;
                        goto err_failed_to_open_file;
                }
        } else {
                if (find_any_at_port(&at_port) != EXIT_SUCCESS) {
                        printf("Failed to open device\n");
                        ret = EXIT_FAILURE;
                        goto err_failed_to_open_file;
                }
        }
        setup_at_port(&at_port);


        struct cJSON *responses = cJSON_CreateArray();

        if (args.at_cmds != NULL) {
                if (execute_cmds(&at_port, args.at_cmds, responses) != EXIT_SUCCESS) {
                        printf("Failed to execute commands\n");
                }
        }
        struct modem modem = { 0 };
        if (get_modem(at_port.usb_info.vendor_id, at_port.usb_info.product_id, &modem) != EXIT_SUCCESS) {
                printf("Failed to find modem\n");
        }
        for (int i = 0; i < __ACTIONS_MAX; i++) {
                if (!args.actions[i]) {
                        continue;
                }
                struct action *action = &modem.actions[i];
                if (action->at_cmd == NULL || strnlen(action->at_cmd, AT_CMD_MAX) == 0 || action->parser == NULL) {
                        printf("Unsupported action\n"); // TODO: json response
                        continue;
                }
                struct cJSON *at_resp = at_execute(&at_port, action->at_cmd, strlen(action->at_cmd));
                if (at_resp == NULL) {
                }
                struct cJSON *parser_resp = NULL;
                action->parser(at_resp, &parser_resp);
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
        close(at_port.fd);
err_failed_to_open_file:
err_failed_argp:
        if (args.at_cmds) {
                cJSON_Delete(args.at_cmds);
        }
        return ret;
}
