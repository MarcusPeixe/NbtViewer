#pragma once

#include <stdint.h>

// colours
#define _CLEAR "\033[0m"
#define _ERR   "\033[31m"
#define _OK    "\033[32m"
#define _INFO  "\033[33m"

#define _STR   "\033[33m"
#define _VAL   "\033[32m"
#define _TYPE  "\033[36m"
#define _PUNCT "\033[37m"

extern uint8_t colours;
extern const char *type_strings[];
extern void (*print_functions[])(Tag_t *);

void increase_indentation();
void decrease_indentation();
void indent_line();
void new_line();
void space();

void print_tag_byte(Tag_t *ptr);
void print_tag_short(Tag_t *ptr);
void print_tag_int(Tag_t *ptr);
void print_tag_long(Tag_t *ptr);
void print_tag_float(Tag_t *ptr);
void print_tag_double(Tag_t *ptr);
void print_tag_byte_array(Tag_t *ptr);
void print_tag_string(Tag_t *ptr);
void print_tag_list(Tag_t *ptr);
void print_tag_compound(Tag_t *ptr);
void print_tag_int_array(Tag_t *ptr);
void print_tag_long_array(Tag_t *ptr);
void print_named_tag(Named_tag_t *tag);
void print_nbt_tag(Named_tag_t *tag);
