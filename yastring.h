#ifndef YASTRING_H_SENTINEL
#define YASTRING_H_SENTINEL
#include <stdlib.h>
void fill_string_with_gaps(char *str, int begin, int end);

void init_string(char **string, int len);

void delete_string(char **string);

int get_string_len(const char *string);

void copy_string(char *des, const char *srs);

void increase_string(char **string, int inc);

int check_string_equality(const char *str_f, const char *str_s);

#endif
