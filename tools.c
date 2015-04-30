#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

size_t
char_of_index(const char * string, int ch){
    if(NULL == string) return -1;
    size_t i;
    for(i = 0; 0 != string[i]; ++i){
        if(ch == string[i]){
            return i;
        }
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
    size_t idx, s = 0, p = 0;

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
        if((idx = char_of_index(pat, src[s-p+p_len])) < 0){
            s = s-p+p_len+1;
        }else{
            s = s-p+p_len-idx;
        }
        p = 0;
    }
    return -1;
}

int
not_contain(char c, const char * str){
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