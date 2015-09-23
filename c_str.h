#ifndef C_STR_H
#define C_STR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool strip(char *str);
bool split_strip(char *str, char *split_chr);
bool split(char *result_str, char * origin_str, char split_chr, int split_index);
char *str_split(char * origin_str, char split_chr, int split_index);
bool memncpy(char *result_str, char *origin_str, int start_index, int cpy_num);


#endif // C_STR_H

