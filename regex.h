#ifndef _REGEX_H
#define _REGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
#define RDT_LOOP 4

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

size_t char_of_index(const char * string, int ch){
    if(NULL == string) return -1;
    size_t i;
    for(i = 0; 0 != string[i]; ++i){
        if(ch == string[i]){
            return i;
        }
    }
    return -1;
}

char *
get_sub_string(const char * src, size_t start, size_t len){
    if(NULL == src || 0 == len || (start+len) > strlen(src)){
        return NULL;
    }
    char * str = Malloc(len+1);
    snprintf(str, len+1, "%s", src+start);
    return str;
}

int
string_to_integer(const char *string, size_t left, size_t right){
    if(left > right){
        return -1;
    }else if(left == right){
        return 0;
    }
    int i, res = 0;
    for(i = left; i < right; ++i){
        if(! isdigit(string[i])){
            return -1;
        }
        res *= 10;
        res += string[i] - '0';
    }
    return res;
}

/**
   tools end
*/

#define start(match) ((match)->start)
#define length(match) ((match)->length)
#define src_str(match) ((match)->src_str)
#define flag(regex) ((regex)->data.flag)
#define type(regex) ((regex)->data.type)
#define child_reg(regex) ((regex)->child_reg)
#define right_reg(regex) ((regex)->right_reg)
#define sim_str(regex) ((regex)->data.string.sim_str)
#define sin_char(regex) ((regex)->data.string.sin_char)
#define left(regex) ((regex)->data.string.scope.left)
#define right(regex) ((regex)->data.string.scope.right)
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
    int stat = right_reg_handle(cur_reg, string, offset);
    if(M_FAIL == stat || off != match.start){
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
DF_SINGLE(const struct regex *cur_reg, const char *string, size_t *offset){
    size_t off = *offset;
    if(0 != string[off]){
        if(RDF_NOT == flag(cur_reg)){
            if(! (RDT_SINGLE == type(cur_reg) && string[off] != sin_char(cur_reg))){
                return M_FAIL;
            }
        }else{
            if(! (RDT_ANYKEY == type(cur_reg)
                    || (RDT_SINGLE == type(cur_reg)
                        && string[off] != sin_char(cur_reg))
                    || (RDT_SCOPE == type(cur_reg)
                        && (left(cur_reg) <= string[off]
                            && string[off] <= right(cur_reg))))){
                return M_FAIL;
            }
        }
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

int
DF_SIMPLE(const struct regex *cur_reg, const char *string, size_t *offset){
    int stat;
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
        if(NULL == right_reg(cur_reg)){
            break;
        }
        stat = right_reg_handle(cur_reg, string, offset);
    }while(M_COMPLETE != stat);
    match.start = start;
    match.length += len;
    return M_SUCCESS;
}

int
DF_STAR(const struct regex *cur_reg, const char *string, size_t *offset){
    int stat = 1;
    size_t off;
    while(1){
        if(0 == string[*offset]){
            if(0 == stat) return M_FAIL;
            if(DF_FINISH == handle(right_reg(cur_reg))){
                return M_COMPLETE;
            }
            return M_SUCCESS;
        }
        off = *offset;
        if(DF_FINISH != handle(right_reg(cur_reg))){
            if(M_COMPLETE == right_reg_handle(cur_reg, string, offset)){
                return M_COMPLETE;
            }else{
                stat = 0;
            }
        }
        *offset = off;
        if(M_FAIL == child_reg_handle(cur_reg, string, offset)){
            return M_SUCCESS;
        }
    }
    return M_FAIL;
}

int
DF_PLUS(const struct regex *cur_reg, const char *string, size_t *offset){
    int stat;
    stat = child_reg_handle(cur_reg, string, offset);
    if(M_SUCCESS == stat){
        return DF_STAR(cur_reg, string, offset);
    }
    return M_FAIL;
}

int
DF_SCOPE(const struct regex *cur_reg, const char *string, size_t *offset){
    int count;
    for(count = 1; ; ++count){
        if(M_FAIL == child_reg_handle(cur_reg, string, offset)){
            if(count <= left(cur_reg)){
                return M_FAIL;
            }
            break;
        }else{
            if(0 == right(cur_reg) || count >= right(cur_reg)){
                break;
            }
        }
    }
    return right_reg_handle(cur_reg, string, offset);
}

int
DF_ONCE(const struct regex *cur_reg, const char *string, size_t *offset){
    if(M_FAIL == child_reg_handle(cur_reg, string, offset)){
        return M_FAIL;
    }
    return right_reg_handle(cur_reg, string, offset);
}

int
DF_OR(const struct regex *cur_reg, const char *string, size_t *offset){
    size_t off = *offset;
    if(M_FAIL == child_reg_handle(cur_reg, string, &off)){
        if(NULL == right_reg(cur_reg)) return M_FAIL;
        if(M_FAIL == right_reg_handle(cur_reg, string, &off)){
            return M_FAIL;
        }
    }
    *offset = off;
    return M_SUCCESS;
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
do_loop(const char *string, size_t *offset){
    size_t off = *offset;
    struct regex *parent_reg = Calloc(1, sizeof(struct regex));
    ++(*offset);
    if('+' == string[off]){
        handle(parent_reg) = DF_PLUS;
    }else if('*' == string[off]){
        handle(parent_reg) = DF_STAR;
    }else if('{' == string[off]){
        size_t mid, end, off = *offset;
        mid = off + char_of_index(string+off, ',');
        end = off + char_of_index(string+off, '}');
        handle(parent_reg) = DF_SCOPE;
        type(parent_reg) = RDT_LOOP;
        left(parent_reg) = string_to_integer(string, *offset, mid);
        right(parent_reg) = string_to_integer(string, mid+1, end);
        *offset = end+1;
    }else{
        --(*offset);
        handle(parent_reg) = DF_ONCE;
    }
    return parent_reg;
}

struct regex *
do_parentheses(const char *string, size_t *offset){
    if('|' == string[*offset]) return NULL;
    int i, end, len, off = *offset;
    struct regex *regex = NULL, *cur_reg = NULL, *temp = NULL;
    len = char_of_index(string+off, ')');
    end = off + len;
    for(i = off; i < end; ++i){
        if('|' == string[i]){
            temp = Calloc(1, sizeof(struct regex));
            handle(temp) = DF_SIMPLE;
            type(temp) = RDT_SIMPLE;
            sim_str(temp) = get_sub_string(string, off, i-off);
            off = i+1;
            if(NULL == regex){
                regex = Calloc(1, sizeof(struct regex));
                handle(regex) = DF_OR;
                child_reg(regex) = temp;
                cur_reg = regex;
            }else{
                right_reg(cur_reg) = Calloc(1, sizeof(struct regex));
                cur_reg = right_reg(cur_reg);
                handle(cur_reg) = DF_OR;
                child_reg(cur_reg) = temp;
            }
        }
    }
    if(NULL == regex){
        regex = Calloc(1, sizeof(struct regex));
        handle(regex) = DF_SIMPLE;
        type(regex) = RDT_SIMPLE;
        sim_str(regex) = get_sub_string(string, *offset, len);
    }else{
        temp = Calloc(1, sizeof(struct regex));
        handle(temp) = DF_SIMPLE;
        type(temp) = RDT_SIMPLE;
        sim_str(temp) = get_sub_string(string, off, len-off);
        right_reg(cur_reg) = temp;
    }
    *offset = end+1;
    return regex;
}

struct regex *
do_bracket(const char *string, size_t *offset){
    int i, off, len, end, flag = RDF_NORMAL;
    if('^' == string[*offset]){
        ++(*offset);
        flag = RDF_NOT;
    }
    if(']' == string[*offset]) return NULL;
    struct regex *regex = NULL, *cur_reg = NULL, *temp = NULL;

    off = *offset;
    len = char_of_index(string+off, ']');
    end = off + len;

    for(i = off; i < end; ++i){
        if('-' == string[i]){
            temp = Calloc(1, sizeof(struct regex));
            handle(temp) = DF_SINGLE;
            type(temp) = RDT_SCOPE;
            flag(temp) = flag;
            left(temp) = string[i-1];
            right(temp) = string[i+1];
            if(NULL == regex){
                regex = Calloc(1, sizeof(struct regex));
                handle(regex) = DF_OR;
                child_reg(regex) = temp;
                cur_reg = regex;
            }else{
                right_reg(cur_reg) = Calloc(1, sizeof(struct regex));
                cur_reg = right_reg(cur_reg);
                handle(cur_reg) = DF_OR;
                child_reg(cur_reg) = temp;
            }
            ++i;
        }else{
            if('-' == string[i+1]){
                continue;
            }
            temp = Calloc(1, sizeof(struct regex));
            handle(temp) = DF_SINGLE;
            type(temp) = RDT_SINGLE;
            flag(temp) = flag;
            sin_char(temp) = string[i];
            if(NULL == regex){
                regex = Calloc(1, sizeof(struct regex));
                handle(regex) = DF_OR;
                child_reg(regex) = temp;
                cur_reg = regex;
            }else{
                right_reg(cur_reg) = Calloc(1, sizeof(struct regex));
                cur_reg = right_reg(cur_reg);
                handle(cur_reg) = DF_OR;
                child_reg(cur_reg) = temp;
            }
        }
    }
    *offset = end+1;
    return regex;
}


int
parse_regex(struct regex **cur_reg, const char *reg_str, size_t *offset){
    if(NULL == reg_str){
        return -1;
    }
    int off = *offset;
    struct regex *parent_reg;
    *cur_reg = Calloc(1, sizeof(struct regex));

    if(0 == reg_str[off]){
        handle(*cur_reg) = DF_FINISH;
        return 0;
    }else if(0 == off && '^' == reg_str[0]){
        handle(*cur_reg) = DF_BEGIN;
        ++(*offset);
        parse_regex(&right_reg(*cur_reg), reg_str, offset);
        return 0;
    }else if('$' == reg_str[off]){
        if(0 != reg_str[off+1]){
            return -1;
        }
        handle(*cur_reg) = DF_END;
        return 0;
    }else if('.' == reg_str[off]){
        ++(*offset);
        type(*cur_reg) = RDT_ANYKEY;
        handle(*cur_reg) = DF_SINGLE;
    }else if('(' == reg_str[off]){
        ++(*offset);
        *cur_reg = do_parentheses(reg_str, offset);
        if(NULL == *cur_reg) return -1;
    }else if('[' == reg_str[off]){
        ++(*offset);
        *cur_reg = do_bracket(reg_str, offset);
        if(NULL == *cur_reg) return -1;
    }else{
        while(not_contain(reg_str[off], "^$.()[]{}+*") && 0 != reg_str[off]){
            ++off;
        }
        if(! not_contain(reg_str[off], ")]}")) return -1;
        handle(*cur_reg) = DF_SIMPLE;
        type(*cur_reg) = RDT_SIMPLE;
        sim_str(*cur_reg) = get_sub_string(reg_str, (*offset), off-(*offset));
        *offset = off;
    }
    parent_reg = do_loop(reg_str, offset);
    child_reg(parent_reg) = *cur_reg;
    *cur_reg = parent_reg;
    parse_regex(&right_reg(*cur_reg), reg_str, offset);
    return 0;
}

struct regex *
init_regex(const char *reg_str){
    if(NULL == reg_str || 0 == reg_str[0]) return NULL;
    size_t off = 0;
    struct regex *regex = NULL;
    if(parse_regex(&regex, reg_str, &off) < 0){
        free_regex(regex);
        return NULL;
    }
    return regex;
}

struct match *
match_regex(struct regex *regex, const char *src_str, size_t start){
    if(NULL == regex || NULL == src_str) return NULL;
    match.src_str = src_str;
    match.start = MS_NULL;
    int stat = this_handle(regex, src_str, &start);
    if(M_COMPLETE == stat || M_SUCCESS == stat){
        return &match;
    }
    return NULL;
}

/**
   principal end
*/

#endif