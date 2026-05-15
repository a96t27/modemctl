#ifndef TEXT_LINE_H
#define TEXT_LINE_H

#define TEXT_LINE_MAX 255

struct text_line {
        char line[TEXT_LINE_MAX];
        int len;
        struct text_line *next;
};

struct text_line *create_line(int str_size, char *str);
void free_text_lines(struct text_line **lines);

#endif