#ifndef _REGEX_H
#define _REGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

typedef struct match * (*deal_func)(const void *regex, const char *src_str, size_t offset);

#define MS_NULL -1

struct match{
    size_t start;
    size_t length;
    const char * src_str;
};

#define RDT_SIMPLE 0
#define RDT_SCOPE 1

#define RDF_NORMAL 0
#define RDF_NOT 1

struct data{
    int type;
    int flag;
    union{
        char *sim_str;
        struct {
            size_t left;
            size_t right;
        } scope;
    } string;
};

struct regex{
    deal_func handle;
    struct regex *next_reg;
    struct regex *right_reg;
    struct data data;
};

/**
   tools start
 */

void *
Malloc(size_t size){
    void *ptr = malloc(size);
    if(NULL == ptr){
        perror("malloc error");
        exit(1);
    }
    return ptr;
}

static size_t
indexOf(const char *src, char c){
    if(NULL == src){
        return -1;
    }
    size_t i = 0;
    while(src[i]){
        if(c == src[i]){
            return i;
        }
        ++i;
    }
    return -1;
}

size_t
sunday(const char *src, const char *pat)
{
    if(NULL == src || NULL == pat){
        return -1;
    }
    size_t s_len = strlen(src);
    size_t p_len = strlen(pat);
    size_t idx;
    size_t s = 0, p = 0;

    while(s < s_len && p < p_len)
    {
        while(src[s] == pat[p]){
            ++s, ++p;
            if(p >= p_len){
                return s-p;
            }else if(s >= s_len){
                return -1;
            }
        }
        if((idx = indexOf(pat, src[s-p+p_len])) < 0){
            s = s-p+p_len+1;
        }else{
            s = s-p+p_len-idx;
        }
        p = 0;
    }
    return -1;
}

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
get_scope_string(const char * src, size_t start, size_t len){
    if(NULL == src || 0 == len || (start+len) > strlen(src)){
        return NULL;
    }
    char * str = Malloc(len+1);
    snprintf(str, len+1, "%s", src+start);
    return str;
}

/**
   tools end
 */

#define start(match) ((match)->start)
#define length(match) ((match)->length)
#define src_str(match) ((match)->src_str)
#define flag(regex) ((regex)->data.flag)
#define type(regex) ((regex)->data.type)
#define next_reg(regex) ((regex)->next_reg)
#define right_reg(regex) ((regex)->right_reg)
#define sim_str(regex) ((regex)->data.string.sim_str)
#define handle(regex) ((regex)->handle)
#define next_reg_handle(regex) (handle(next_reg(regex)))
#define right_reg_handle(regex) (handle(right_reg(regex)))


/**
   handles start
 */

static jmp_buf jmp_match_regex;
static struct match match;

struct match *
DF_BEGIN(const struct regex *cur_reg, const char *string, size_t offset){
    size_t off = offset;
    struct match *m = next_reg_handle(cur_reg) (next_reg(cur_reg), string, offset);
    if(NULL == m || off != start(m)){
        longjmp(jmp_match_regex, 1);
    }
    return m;
}

struct match *
DF_END(const struct regex *cur_reg, const char *string, size_t offset){
    if('\0' == string[offset] || '\n' == string[offset]){
        return &match;
    }
    return NULL;
}

struct match *
DF_SIMPLE(const struct regex *cur_reg, const char *string, size_t offset){
    struct match *m;
    size_t len, start = match.start;
    do{
        int first = sunday(string+offset, sim_str(cur_reg));
        if(-1 == first){
            longjmp(jmp_match_regex, 1);
        }
        if(MS_NULL == match.start){
            start = first + offset;
        }else if(0 != first){
            longjmp(jmp_match_regex, 1);
        }
        len = strlen(sim_str(cur_reg));
        offset += first + strlen(sim_str(cur_reg));
        m = next_reg_handle(cur_reg) (next_reg(cur_reg), string, offset);
    }while(NULL == m);
    match.start = start;
    match.length += len;
    return &match;
}

struct match *
DF_NULL(const struct regex *cur_reg, const char *string, size_t offset){
    return &match;
}

/**
   handles end
 */


/**
   principal start
 */

static jmp_buf jmp_init_regex;

void
free_regex(struct regex *reg){
    if(NULL == reg) return;
    struct regex * next = next_reg(reg);
    struct regex * right= right_reg(reg);
    free_regex(next);
    free_regex(right);
    if(RDT_SIMPLE == type(reg)){
        free(sim_str(reg));
    }
    free(reg);
}


void
parse_regex(const char *reg_str, size_t off, struct regex **cur_reg){
    if(NULL == reg_str){
        longjmp(jmp_init_regex, 1);
    }
    *cur_reg = (struct regex *)Malloc(sizeof(struct regex));
    bzero(*cur_reg, sizeof(struct regex));

    if('\0' == reg_str[off]){
        handle(*cur_reg) = DF_NULL;
    }else if(0 == off && '^' == reg_str[0]){
        handle(*cur_reg) = DF_BEGIN;
        parse_regex(reg_str, 1, &(next_reg(*cur_reg)));
    }else if('$' == reg_str[off]){
        if('\0' != reg_str[off+1]){
            longjmp(jmp_init_regex, 1);
        }
        handle(*cur_reg) = DF_END;
    }else{
        int i = off;
        while(not_contain(reg_str[i], "^$") && '\0' != reg_str[i]){
            ++i;
        }
        handle(*cur_reg) = DF_SIMPLE;
        type(*cur_reg) = RDT_SIMPLE;
        flag(*cur_reg) = RDF_NORMAL;
        sim_str(*cur_reg) = get_scope_string(reg_str, off, i-off);
        parse_regex(reg_str, i, &(next_reg(*cur_reg)));
    }
}

struct regex *
init_regex(const char *reg_str){
    if(NULL == reg_str || '\0' == reg_str[0]) return NULL;

    struct regex *regex = NULL;
    if(0 != setjmp(jmp_init_regex)){
        free_regex(regex);
        return NULL;
    }
    parse_regex(reg_str, 0, &regex);
    return regex;
}

struct match *
match_regex(struct regex *regex, const char *src_str, size_t start){
    if(NULL == regex || NULL == src_str) return NULL;
    if(0 != setjmp(jmp_match_regex)){
        return NULL;
    }
    match.src_str = src_str;
    match.start = MS_NULL;
    return handle(regex) (regex, src_str, start);
}

/**
   principal end
 */


#endif