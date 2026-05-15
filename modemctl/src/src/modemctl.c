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
#include <text_lines.h>
#include <cjson/cJSON.h>
#include <commands.h>

#define DEFAULT_DEVICE "/dev/ttyUSB2"

int running = 1;

void sig_handler(int sigint)
{
        (void)sigint;
        running = 0;
}


static error_t parse_opt(int key, char *arg, struct argp_state *state);
struct Arguments {
        int all;
        int imei;
        int opertaor;
        int signal;
        int cell;
        int band;
        int status;
        int json;
        char device[PATH_MAX];
        int debug;
        int watch;
};

static char doc[] = "Modemctl - command line tool to control modems";
static char args_doc[] = "";
static struct argp_option options[] = {
        { "imei", 'i', 0, 0, "Get IMEI", 0 },
        { "status", 'u', 0, 0, "Get modem status", 0 },
        { "device", 'd', DEFAULT_DEVICE, 0, "Set AT command port", 0 },
        { "debug", 'g', 0, 0, "Toggle debug mode", 0 },
        // { "all", 'a', 0, 0, "get all", 0},
        // { "operator", 'o', 0 ,0, "Get operator", 0 },
        // { "signal", 's', 0, 0, "Get signal level", 0 },
        // { "cell", 'c', 0, 0, "Get cell", 0 },
        // { "band", 'b', 0, 0, "Get band", 0 },
        // { "json", 'j', 0, 0, "Get json", 0},
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
        if (args.imei) {
                struct cJSON *iemi_resp = get_imei(fd);
                char *txt = cJSON_Print(iemi_resp);
                printf("%s\n", txt);
                cJSON_Delete(iemi_resp);
                free(txt);
        }
        close(fd);
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
                arguments->imei = 1;
                break;
        case 'u':
                arguments->status = 1;
                break;
        case 'g':
                arguments->debug = 1;
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

