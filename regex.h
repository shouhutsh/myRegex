#ifndef _REGEX_H
#define _REGEX_H

#include <stdlib.h>

typedef int (*deal_func)(const void *regex, const char *src_str, size_t *offset);

struct match{
    size_t start;
    size_t length;
    const char * src_str;
};

struct data{
    int type;
    int flag;
    union{
        char *sim_str;
        char sin_char;
        struct {
            size_t left;
            size_t right;
        } scope;
    } string;
};

struct regex{
    deal_func handle;
    struct regex *child_reg;
    struct regex *right_reg;
    struct data data;
};

struct regex *init_regex(const char *reg_str);
struct match *match_regex(struct regex *regex, const char *src_str, size_t start);
void free_regex(struct regex *reg);
char *get_sub_string(const char * src, size_t start, size_t len);

#endif