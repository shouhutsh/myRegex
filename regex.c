#include "regex.h"

int
main(int argc, char * argv[])
{
    if(3 != argc){
        printf("Usage: %s \"<regex string>\" \"<source string>\"", argv[0]);
        exit(0);
    }

    char * str;
    struct match *match;
    struct regex *regex;

    regex = init_regex(argv[1]);
    if(NULL == regex){
        perror("init_regex error");
        exit(0);
    }
    match = match_regex(regex, argv[2], 0);
    if(NULL == match){
        perror("match_regex error");
        exit(0);
    }

    str = get_string(match);
    printf("match: %s\n", str);
    free(str);
    free_regex(regex);

    return 0;
}