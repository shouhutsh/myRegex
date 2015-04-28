#ifndef _REGEX_H
#define _REGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

typedef int (*deal_func)(const void *regex, const char *src_str, size_t *offset);

#define MS_NULL -1

struct match{
    size_t start;
    size_t length;
    const char * src_str;
};

#define RDT_SIMPLE 0
#define RDT_SCOPE 1
#define RDT_ANYKEY 2
#define RDT_SINGLE 3

#define RDF_NORMAL 0
#define RDF_NOT 1

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

void *
Calloc(size_t nmemb, size_t size){
    void *ptr = calloc(nmemb, size);
    if(NULL == ptr){
        perror("calloc error");
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
#define flag(regex) ((regex)->data.flag)
#define type(regex) ((regex)->data.type)
#define src_str(match) ((match)->src_str)
#define child_reg(regex) ((regex)->child_reg)
#define right_reg(regex) ((regex)->right_reg)
#define sim_str(regex) ((regex)->data.string.sim_str)
#define sin_char(regex) ((regex)->data.string.sin_char)
#define handle(regex) ((regex)->handle)
#define this_handle(regex, string, offset) (handle(regex) ((regex), (string), (offset)))
#define child_reg_handle(regex, string, offset) (this_handle((child_reg(regex)), (string), (offset)))
#define right_reg_handle(regex, string, offset) (this_handle((right_reg(regex)), (string), (offset)))


/**
   handles start
 */

static struct match match;

#define M_FAIL (0)
#define M_SUCCESS (1)
#define M_COMPLETE (2)

int
DF_BEGIN(const struct regex *cur_reg, const char *string, size_t *offset){
    size_t off = *offset;
    int status = right_reg_handle(cur_reg, string, offset);
    if(M_FAIL == status || off != match.start){
        return M_FAIL;
    }
    return M_COMPLETE;
}

int
DF_END(const struct regex *cur_reg, const char *string, size_t *offset){
    if(0 == string[*offset] || '\n' == string[*offset]){
        return M_COMPLETE;
    }
    return M_FAIL;
}

int
DF_FINISH(const struct regex *cur_reg, const char *string, size_t *offset){
    return M_COMPLETE;
}

int
DF_SIMPLE(const struct regex *cur_reg, const char *string, size_t *offset){
    int status;
    size_t off = *offset;
    size_t len, start = match.start;
    do{
        int first = sunday(string+off, sim_str(cur_reg));
        if(-1 == first){
            return M_FAIL;
        }
        if(MS_NULL == match.start){
            start = first + off;
        }else if(0 != first){
            return M_FAIL;
        }
        len = strlen(sim_str(cur_reg));
        *offset += first + len;
        status = right_reg_handle(cur_reg, string, offset);
    }while(M_COMPLETE != status);
    match.start = start;
    match.length += len;
    return M_COMPLETE;
}

int
DF_STAR(const struct regex *cur_reg, const char *string, size_t *offset){
    int status;
    size_t off;
    while(1){
        if(0 == string[*offset]){
            if(DF_FINISH == handle(right_reg(cur_reg))){
                return M_COMPLETE;
            }
            return M_SUCCESS;
        }
        off = *offset;
        if(DF_FINISH != handle(right_reg(cur_reg))
            && M_COMPLETE == right_reg_handle(cur_reg, string, offset)){
            return M_COMPLETE;
        }
        *offset = off;
        if(M_FAIL == child_reg_handle(cur_reg, string, offset)){
            return M_SUCCESS;
        }
    }
}

int
DF_PLUS(const struct regex *cur_reg, const char *string, size_t *offset){
    int status;
    size_t off;
    status = child_reg_handle(cur_reg, string, offset);
    if(M_SUCCESS == status){
        return DF_STAR(cur_reg, string, offset);
    }
    return M_FAIL;
}

int
DF_SINGLE(const struct regex *cur_reg, const char *string, size_t *offset){
    size_t off = *offset;
    if(0 != string[off]
        && (RDT_ANYKEY == type(cur_reg) || string[off] == sin_char(cur_reg))){
        if(MS_NULL == match.start){
            match.start = off;
        }
        ++(*offset);
        ++(match.length);
        if(NULL == right_reg(cur_reg)){
            return M_SUCCESS;
        }
        return right_reg_handle(cur_reg, string, offset);
    }
    return M_FAIL;
}

/**
   handles end
 */


/**
   principal start
 */

void
free_regex(struct regex *reg){
    if(NULL == reg) return;
    struct regex * next = child_reg(reg);
    struct regex * right= right_reg(reg);
    if(next != reg && NULL != next){
        free_regex(next);
    }
    if(right != reg && NULL != right){
        free_regex(right);
    }
    if(RDT_SIMPLE == type(reg)){
        free(sim_str(reg));
    }
    free(reg);
}

struct regex *
parse_regex(const char *reg_str, size_t *offset, int end){
    if(NULL == reg_str){
        return NULL;
    }
    int off = *offset;
    struct regex *cur_reg = Calloc(1, sizeof(struct regex));

    if(0 == reg_str[off]){
        handle(cur_reg) = DF_FINISH;
    }else if(0 == off && '^' == reg_str[0]){
        handle(cur_reg) = DF_BEGIN;
        ++(*offset);
        right_reg(cur_reg) = parse_regex(reg_str, offset, 0);
    }else if('$' == reg_str[off]){
        if(0 != reg_str[off+1]){
            return NULL;
        }
        handle(cur_reg) = DF_END;
    }else if('.' == reg_str[off]){
        type(cur_reg) = RDT_ANYKEY;
        handle(cur_reg) = DF_SINGLE;
        if('+' == reg_str[off+1]){
            ++(*offset);
            struct regex *parent_reg = Calloc(1, sizeof(struct regex));
            handle(parent_reg) = DF_PLUS;
            child_reg(parent_reg) = cur_reg;
            cur_reg = parent_reg;
        }else if('*' == reg_str[off+1]){
            ++(*offset);
            struct regex *parent_reg = Calloc(1, sizeof(struct regex));
            handle(parent_reg) = DF_STAR;
            child_reg(parent_reg) = cur_reg;
            cur_reg = parent_reg;
        }
        ++(*offset);
        right_reg(cur_reg) = parse_regex(reg_str, offset, 0);
    }

    else{
        while(not_contain(reg_str[off], "^.$") && 0 != reg_str[off]){
            ++off;
        }
        handle(cur_reg) = DF_SIMPLE;
        type(cur_reg) = RDT_SIMPLE;
        flag(cur_reg) = RDF_NORMAL;
        sim_str(cur_reg) = get_scope_string(reg_str, (*offset), off-(*offset));
        *offset = off;
        right_reg(cur_reg) = parse_regex(reg_str, offset, 0);
    }
    return cur_reg;
}

struct regex *
init_regex(const char *reg_str){
    if(NULL == reg_str || 0 == reg_str[0]) return NULL;
    size_t off = 0;
    return parse_regex(reg_str, &off, 0);
}

struct match *
match_regex(struct regex *regex, const char *src_str, size_t start){
    if(NULL == regex || NULL == src_str) return NULL;
    match.src_str = src_str;
    match.start = MS_NULL;
    if(M_COMPLETE == this_handle(regex, src_str, &start)){
        return &match;
    }
    return NULL;
}

/**
   principal end
 */


#endif