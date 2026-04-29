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

#define ARGSIZE 255

// int running = 1;

// void sig_handler(int sigint)
// {
//         running = 0;
// }


// static error_t parse_opt(int key, char *arg, struct argp_state *state);
// struct Arguments {
//         int all;
//         int imei;
//         int opertaor;
//         int signal;
//         int cell;
//         int band;
//         int status;
//         int json;
//         char device[ARGSIZE];
//         int debug;
//         int watch;
// };

// static char doc[] = "Modemctl - command line tool to control modems";
// static char args_doc[] = "";
// static struct argp_option options[] = {
//         { "imei", 'i', 0, 0, "Get IMEI", 0 },
//         { "status", 'u', 0, 0, "Get status", 0 },
//         { "device", 'd', "DEVICE", 0, "Get device", 0 },
//         { "debug", 'g', 0, 0, "Get debug", 0 },
//         // { "all", 'a', 0, 0, "get all", 0},
//         // { "operator", 'o', 0 ,0, "Get operator", 0 },
//         // { "signal", 's', 0, 0, "Get signal level", 0 },
//         // { "cell", 'c', 0, 0, "Get cell", 0 },
//         // { "band", 'b', 0, 0, "Get band", 0 },
//         // { "json", 'j', 0, 0, "Get json", 0},
//         // { "watch", 'w', 0, 0, "keep watching", 0 },
//         { 0 },
// };
// static struct argp _argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };


int main(int argc, char **argv)
{
        // signal(SIGINT, sig_handler);
        // signal(SIGTERM, sig_handler);
        // signal(SIGQUIT, sig_handler);
        // signal(SIGABRT, SIG_IGN);
        // signal(SIGILL, SIG_IGN);
        // signal(SIGFPE, SIG_IGN);
        // signal(SIGHUP, SIG_IGN);
        // signal(SIGIO, SIG_IGN);
        // signal(SIGUSR1, SIG_IGN);
        // signal(SIGUSR2, SIG_IGN);

        // struct Arguments args = { 0 };

        // argp_parse(&_argp, argc, argv, 0, 0, &args);

        int fd = at_get_port();
        at_setup_port(fd);
        char output_buf[1024];
        for (int i = 1; i < argc; i++) {
                at_execute(fd, strlen(argv[i]), argv[i], sizeof(output_buf), output_buf);
                printf("%s\n", output_buf);
        }
        close(fd);
        return 0;
}




// static error_t parse_opt(int key, char *arg, struct argp_state *state)
// {
//         struct Arguments *arguments = state->input;
//         switch (key) {
//         case 'd':
//                 memcpy(arguments->device, arg, ARGSIZE);
//                 arguments->device[ARGSIZE - 1] = '\0';
//                 break;
//         case 'i':
//                 arguments->imei = 1;
//                 break;
//         case 'u':
//                 arguments->status = 1;
//                 break;
//         case 'g':
//                 arguments->debug = 1;
//                 break;
//         case ARGP_KEY_ARG:
//                 if (state->arg_num > 0)
//                         argp_usage(state);
//                 break;
//         case ARGP_KEY_END:
//                 if (state->arg_num > 0)
//                         argp_usage(state);
//                 break;
//         default:
//                 return ARGP_ERR_UNKNOWN;
//                 break;
//         }
//         return 0;
// }

