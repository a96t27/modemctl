#include <text_lines.h>
#include <stdlib.h>
#include <string.h>

struct text_line *create_line(int str_size, char *str)
{
        struct text_line *line = (struct text_line *)malloc(sizeof(struct text_line));
        if (line == NULL) {
                return NULL;
        }
        line->next = NULL;
        line->len = str_size < TEXT_LINE_MAX ? str_size : TEXT_LINE_MAX - 1;
        strncpy(line->line, str, line->len);
        line->line[line->len] = 0;
        return line;
}

void free_text_lines(struct text_line **lines)
{
        if (lines == NULL) {
                return;
        }
        struct text_line *temp = *lines;
        struct text_line *to_delete = *lines;
        while (temp != NULL) {
                temp = temp->next;
                free(to_delete);
                to_delete = temp;
        }
        *lines = NULL;
}
