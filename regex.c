#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct match{
    size_t start;
    size_t end;
    char * src_str;
};

struct regex{

};

struct regex *
init_regex(const char *reg_str){
    return NULL;
}

struct match *
match_regex(struct regex *regex, const char *src_str, size_t start, size_t end){
    return NULL;
}

char *
get_string(const struct match * match){
    return NULL;
}

int
main(int argc, char * argv[])
{
    if(3 != argc){
        printf("Usage: %s \"<regex string>\" \"<source string>\"", argv[0]);
    }

    char * str;
    struct match *match;
    struct regex *regex;

    regex = init_regex(argv[1]);
    if(NULL == regex){
        perror("init_regex error");
    }
    match = match_regex(regex, argv[2], 0, 0);
    if(NULL == match){
        perror("match_regex error");
    }

    str = get_string(match);
    printf("match: %s\n", str);
    free(str);

    return 0;
}