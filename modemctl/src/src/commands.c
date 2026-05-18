// #include <commands.h>
// #include <cjson/cJSON.h>
// #include <serial_io.h>
// #include <parser.h>
// #include <stdio.h>
// #include <response.h>

// struct cJSON *get_imei(int fd, int verbose)
// {
//         if (fd < 0) {
//                 return NULL;
//         }
//         char cmd[] = "AT+GSN";
//         char type[] = "imei";
//         struct text_line *lines = at_execute(fd, sizeof(cmd), cmd);
//         if (lines == NULL) {
//                 char msg[] = "Failed to execute AT command";
//                 return create_response(0, type, sizeof(type), msg, sizeof(msg), NULL);
//         }
//         if (verbose) {
//                 PRINT_CMD(cmd);
//                 print_text_lines(lines);
//         }
//         struct cJSON *data = parse_get_imei(lines);
//         if (data == NULL) {
//                 char msg[] = "Failed to parse AT command";
//                 return create_response(0, type, sizeof(type), msg, sizeof(msg), NULL);
//         }
//         free_text_lines(&lines);
//         char msg[] = "Got IMEI code";
//         return create_response(1, type, sizeof(type), msg, sizeof(msg), data);
// }
