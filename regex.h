#ifndef _REGEX_H
#define _REGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct match * (*deal_func)(const void *regex, const char *src_str, size_t offset);

struct match{
    size_t start;
    size_t finish;
    char * src_str;
};

struct regex{
    int flag;
    deal_func handle;
    struct regex *next_reg;
    char * sim_str;
};

/*************/

#define start(match) (match->start)
#define finish(match) (match->finish)
#define src_str(match) (match->src_str)
#define flag(regex) (regex->flag)
#define next_reg(regex) (regex->next_reg)
#define sim_str(regex) (regex->sim_str)
#define handle(regex) (regex->handle)
#define next_reg_handle(regex) (handle(next_reg(regex)))


struct match match;

struct match *
DF_BEGIN(const struct regex *cur_reg, const char *string, size_t offset){
    size_t off = offset;
    struct match *m = next_reg_handle(cur_reg) (next_reg(cur_reg), string, offset);
    if(off != start(m)){
        return NULL;
    }
    return m;
}

struct match *
DF_END(const struct regex *cur_reg, const char *string, size_t offset){
    if('\0' == string[offset] || '\n' == string[offset]){
        match.finish = offset;
        return &match;
    }
    return NULL;
}

struct match *
DF_SIMPLE(const struct regex *cur_reg, const char *string, size_t offset){
    char ch;
    size_t off = offset;
    char *sim_str = sim_str(cur_reg);
    while(0 != (ch = *(sim_str++))){
        if(ch == string[off]){
            ++off;
        }else{
            return NULL;
        }
    }
    return next_reg_handle(cur_reg) (next_reg(cur_reg), string, off);
}

struct match *
DF_NULL(const struct regex *cur_reg, const char *string, size_t offset){
    match.finish = offset;
    return &match;
}

/*************/

int not_contain(char c, const char * str){
    if(NULL == str){
        return -1;
    }
    int i;
    for(i = 0; i < strlen(str); ++i){
        if(c == str[i]){
            return 0;
        }
    }
    return 1;
}

char *
get_string(const struct match * match){
    int len = finish(match) - start(match) + 1;
    char *str = malloc(len + 1);
    snprintf(str, len+1, "%s", src_str(match) + start(match));
    return str;
}

static struct regex *
get_regex(const char *reg_str){
    int i = reg_str[0];
    struct regex *cur_reg = malloc(sizeof(struct regex));
    bzero(cur_reg, sizeof(struct regex));

    if('\0' == i){
        handle(cur_reg) = DF_NULL;
    }else if('$' == i){
        handle(cur_reg) = DF_END;
        if('\0' != reg_str[1]){
            free(cur_reg);
            return NULL;
        }
    }else{
        int i = 0;
        struct match m;
        m.src_str = reg_str;
        m.start = 0;
        while(not_contain(reg_str[i], "^$") && '\0' != reg_str[i]){
            ++i;
        }
        m.finish = i-1;
        handle(cur_reg) = DF_SIMPLE;
        sim_str(cur_reg) = get_string(&m);
        next_reg(cur_reg) = get_regex(reg_str+i);
        if(NULL == next_reg(cur_reg)){
            free(cur_reg);
            return NULL;
        }
    }
    return cur_reg;
}

struct regex *
init_regex(const char *reg_str){
    if(NULL == reg_str || '\0' == reg_str[0]) return NULL;

    if('^' == reg_str[0]){
        struct regex *r = malloc(sizeof(struct regex));
        bzero(r, sizeof(struct regex));
        handle(r) = DF_BEGIN;
        next_reg(r) = get_regex(reg_str+1);
        if(NULL == next_reg(r)){
            free(r);
            return NULL;
        }
        return r;
    }else{
        return get_regex(reg_str);
    }
}

void
free_regex(struct regex *reg){
    struct regex * next = next_reg(reg);
    while(NULL != next){
        free(sim_str(reg));
        free(reg);
        reg = next;
        next = next_reg(next);
    }
}

struct match *
match_regex(struct regex *regex, const char *src_str, size_t start){
    if(NULL == regex || NULL == src_str) return NULL;
    match.src_str = src_str;
    match.start = start;
    return handle(regex) (regex, src_str, start);
}

#endif