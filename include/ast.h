#pragma once

#include <stdint.h>

//// DECLARATIONS AND TYPEDEFS ////

enum TAG_TYPE
{
    TAG_End,        //  0
    TAG_Byte,       //  1
    TAG_Short,      //  2
    TAG_Int,        //  3
    TAG_Long,       //  4
    TAG_Float,      //  5
    TAG_Double,     //  6
    TAG_Byte_Array, //  7
    TAG_String,     //  8
    TAG_List,       //  9
    TAG_Compound,   // 10
    TAG_Int_Array,  // 11
    TAG_Long_Array, // 12
};

typedef struct Tag_s
{
    uint8_t type;
} Tag_t;

extern void (*free_functions[])(Tag_t *);

typedef struct Tag_byte_s
{
    uint8_t type;
    int8_t load;
} Tag_byte_t;

typedef struct Tag_short_s
{
    uint8_t type;
    int16_t load;
} Tag_short_t;

typedef struct Tag_int_s
{
    uint8_t type;
    int32_t load;
} Tag_int_t;

typedef struct Tag_long_s
{
    uint8_t type;
    int64_t load;
} Tag_long_t;

typedef struct Tag_float_s
{
    uint8_t type;
    float load;
} Tag_float_t;

typedef struct Tag_double_s
{
    uint8_t type;
    double load;
} Tag_double_t;

typedef struct Tag_byte_array_s
{
    uint8_t type;
    int8_t *load;
    int32_t length;
} Tag_byte_array_t;

typedef struct Tag_string_s
{
    uint8_t type;
    int8_t *load;
    int16_t length;
} Tag_string_t;

typedef struct Tag_list_s
{
    uint8_t type;
    Tag_t **load;
    int32_t length;
    int8_t list_type;
} Tag_list_t;

typedef struct Named_tag_s
{
    uint8_t type;
    Tag_string_t *name;
    Tag_t *tag;
} Named_tag_t;

typedef struct Tag_compound_s
{
    uint8_t type;
    Named_tag_t **load;
} Tag_compound_t;

typedef struct Tag_int_array_s
{
    uint8_t type;
    int32_t *load;
    int32_t length;
} Tag_int_array_t;

typedef struct Tag_long_array_s
{
    uint8_t type;
    int64_t *load;
    int32_t length;
} Tag_long_array_t;

typedef struct Compound_node_s
{
    Named_tag_t *tag;
    struct Compound_node_s *previous;
} Compound_node_t;

typedef struct List_node_s
{
    Tag_t *tag;
    struct List_node_s *previous;
} List_node_t;

//// FUNCTIONS ////

Tag_t *new_end();
void free_tag_end(Tag_t *ptr);

Tag_byte_t *new_byte(int8_t);
void free_tag_byte(Tag_t *);

Tag_short_t *new_short(int16_t);
void free_tag_short(Tag_t *);

Tag_int_t *new_int(int32_t);
void free_tag_int(Tag_t *);

Tag_long_t *new_long(int64_t);
void free_tag_long(Tag_t *);

Tag_float_t *new_float(float);
void free_tag_float(Tag_t *);

Tag_double_t *new_double(double);
void free_tag_double(Tag_t *);

Tag_byte_array_t *new_byte_array(int32_t);
void free_tag_byte_array(Tag_t *);

Tag_string_t *new_string(int16_t);
void free_tag_string(Tag_t *);

Tag_list_t *new_list(int8_t, int32_t);
void free_tag_list(Tag_t *);

Tag_compound_t *new_compound(Compound_node_t *);
void free_tag_compound(Tag_t *);

Tag_int_array_t *new_int_array(int32_t);
void free_tag_int_array(Tag_t *);

Tag_long_array_t *new_long_array(int32_t);
void free_tag_long_array(Tag_t *);

Named_tag_t *new_named_tag(int8_t, Tag_string_t *, Tag_t *);
void free_named_tag(Named_tag_t *);

void free_nbt_tag(Named_tag_t *tag);

Compound_node_t *new_compound_list();
Compound_node_t *add_compound_node(Compound_node_t *, Named_tag_t *);
Named_tag_t **finalise_compound_list(Compound_node_t *);

List_node_t *new_nodes_list();
List_node_t *add_list_node(List_node_t *, Tag_t *);
Tag_list_t *finalise_nodes_list(int8_t, List_node_t *);
