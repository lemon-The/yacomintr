#include "yastring.h"
void fill_string_with_gaps(char *str, int begin, int end)
{
    int i;
    for(i = begin; i < end-1; i++)
        str[i] = ' ';
    str[i] = 0;
}

void init_string(char **string, int len)
{
    if(*string)
        free(*string);
    *string = malloc(sizeof(char)*(len+1));
    fill_string_with_gaps(*string, 0, len+1);
}

void delete_string(char **string)
{
    if(*string)
        free(*string);
    *string = NULL;
}

int get_string_len(const char *string)
{
    int i;
    if(!string)
        return 0;
    for(i = 0; *string; string++, i++)
        {}
    return i;
}

void copy_string(char *des, const char *srs)
{
    for(; *srs; des++, srs++)
        *des = *srs;
    *des = 0;
}

void increase_string(char **string, int inc)
{
    int len = get_string_len(*string)+1;
    int new_len = inc + len;
    char *new_str = malloc(sizeof(char)*new_len);
    copy_string(new_str, *string);
    fill_string_with_gaps(new_str, len, new_len);
    free(*string);
    *string = new_str;
}

int check_string_equality(const char *str_f, const char *str_s)
{
    if(get_string_len(str_f) != get_string_len(str_s))
        return 0;
    for(; *str_f; str_f++, str_s++)
        if(*str_f != *str_s)
            return 0;
    return 1;
}

