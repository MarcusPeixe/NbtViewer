#pragma once

#include <ast.h>
#include <stdint.h>

//// MACROS ////

#define CHUNK       0x1000
#define TOTAL_TYPES 13

//// STRUCTS ////

typedef struct error_s
{
    struct error_s *previous;
    const char *message;
    int location;
} error_t;

//// DECLARATIONS ////

void raise_error(int, const char *);
void append_error(int, const char *);
error_t *get_error();
void print_error(error_t *);

Named_tag_t *parse_nbt_tag();
Tag_t *parse_any_data();
Tag_string_t *parse_tag_name();
Named_tag_t *parse_named_tag();

Tag_t *parse_TAG_Byte();
Tag_t *parse_TAG_Short();
Tag_t *parse_TAG_Int();
Tag_t *parse_TAG_Long();
Tag_t *parse_TAG_Float();
Tag_t *parse_TAG_Double();
Tag_t *parse_TAG_Byte_Array();
Tag_t *parse_TAG_String();
Tag_t *parse_TAG_List();
Tag_t *parse_TAG_Compound();
Tag_t *parse_TAG_Int_Array();
Tag_t *parse_TAG_Long_Array();