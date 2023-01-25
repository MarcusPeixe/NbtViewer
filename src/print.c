#include <ast.h>
#include <print.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//// VARIABLES ////

int _indent = 0;
uint8_t colours = 0;

const char *type_strings[] = {
    "TAG_End",      "TAG_Byte",      "TAG_Short",      "TAG_Int",    "TAG_Long",
    "TAG_Float",    "TAG_Double",    "TAG_Byte_Array", "TAG_String", "TAG_List",
    "TAG_Compound", "TAG_Int_Array", "TAG_Long_Array",
};

void (*print_functions[])(Tag_t *) = {
    NULL,
    print_tag_byte,
    print_tag_short,
    print_tag_int,
    print_tag_long,
    print_tag_float,
    print_tag_double,
    print_tag_byte_array,
    print_tag_string,
    print_tag_list,
    print_tag_compound,
    print_tag_int_array,
    print_tag_long_array,
};

//// DEFINITIONS ////

void print_tag_end(Tag_t *ptr)
{
    if (colours)
        printf(_ERR "NULL");
    else
        printf("NULL");
}

void print_tag_byte(Tag_t *ptr)
{
    const Tag_byte_t *tag = (Tag_byte_t *) ptr;
    if (colours)
        printf(_VAL "%d" _TYPE "b", tag->load);
    else
        printf("%db", tag->load);
}

void print_tag_short(Tag_t *ptr)
{
    const Tag_short_t *tag = (Tag_short_t *) ptr;
    if (colours)
        printf(_VAL "%d" _TYPE "s", tag->load);
    else
        printf("%ds", tag->load);
}

void print_tag_int(Tag_t *ptr)
{
    const Tag_int_t *tag = (Tag_int_t *) ptr;
    if (colours)
        printf(_VAL "%d", tag->load);
    else
        printf("%d", tag->load);
}

void print_tag_long(Tag_t *ptr)
{
    const Tag_long_t *tag = (Tag_long_t *) ptr;
    if (colours)
        printf(_VAL "%ld" _TYPE "l", tag->load);
    else
        printf("%ldl", tag->load);
}

void print_tag_float(Tag_t *ptr)
{
    const Tag_float_t *tag = (Tag_float_t *) ptr;
    if (colours)
        printf(_VAL "%f" _TYPE "f", tag->load);
    else
        printf("%ff", tag->load);
}

void print_tag_double(Tag_t *ptr)
{
    const Tag_double_t *tag = (Tag_double_t *) ptr;
    if (colours)
        printf(_VAL "%lf" _TYPE "d", tag->load);
    else
        printf("%lfd", tag->load);
}

void print_tag_byte_array(Tag_t *ptr)
{
    const Tag_byte_array_t *tag = (Tag_byte_array_t *) ptr;
    if (colours)
        printf(_PUNCT "[" _TYPE "B" _PUNCT ";");
    else
        printf("[B;");
    space();

    for (int i = 0; i < tag->length; i++) {
        if (i > 0) {
            if (colours)
                printf(_PUNCT ",");
            else
                printf(",");
            space();
        }

        if (colours)
            printf(_VAL "%d" _TYPE "b", tag->load[i]);
        else
            printf("%db", tag->load[i]);
    }

    if (colours)
        printf(_PUNCT "]");
    else
        printf("]");
}

static void print_safe_str(Tag_string_t const *tag)
{
    for (int i = 0; i < tag->length; i++) {
        uint8_t c = tag->load[i];
        if (c == '"' || c == '\\') {
            printf("\\%c", c);
        }
        else if (c >= 0x20 && c < 0x7F) {
            putchar(c);
        }
        else {
            printf("\\%o", c);
        }
    }
}

void print_tag_string(Tag_t *ptr)
{
    const Tag_string_t *tag = (Tag_string_t *) ptr;

    if (colours)
        printf(_STR "\"");
    else
        printf("\"");

    print_safe_str(tag);

    if (colours)
        printf("\"" _CLEAR);
    else
        printf("\"");
}

void print_tag_list(Tag_t *ptr)
{
    const Tag_list_t *tag = (Tag_list_t *) ptr;
    if (colours)
        printf(_PUNCT "[");
    else
        printf("[");

    new_line();
    increase_indentation();
    for (int i = 0; i < tag->length; i++) {
        if (i > 0) {
            if (colours)
                printf(_PUNCT ",");
            else
                printf(",");
            space();
            new_line();
        }
        indent_line();
        print_functions[tag->list_type](tag->load[i]);
    }

    decrease_indentation();
    new_line();
    indent_line();
    if (colours)
        printf(_PUNCT "]");
    else
        printf("]");
}

void print_tag_compound(Tag_t *ptr)
{
    const Tag_compound_t *tag = (Tag_compound_t *) ptr;

    if (colours)
        printf(_PUNCT "{");
    else
        printf("{");

    new_line();
    increase_indentation();
    for (int i = 0; tag->load[i]; i++) {
        if (i > 0) {
            if (colours)
                printf(_PUNCT ",");
            else
                printf(",");
            space();
            new_line();
        }

        indent_line();
        print_named_tag(tag->load[i]);
    }
    new_line();
    decrease_indentation();
    indent_line();

    if (colours)
        printf(_PUNCT "}");
    else
        printf("}");
}

void print_tag_int_array(Tag_t *ptr)
{
    const Tag_int_array_t *tag = (Tag_int_array_t *) ptr;
    if (colours)
        printf(_PUNCT "[" _TYPE "I" _PUNCT ";");
    else
        printf("[I;");
    space();

    for (int i = 0; i < tag->length; i++) {
        if (i > 0) {
            if (colours)
                printf(_PUNCT ",");
            else
                printf(",");
            space();
        }

        if (colours)
            printf(_VAL "%d", tag->load[i]);
        else
            printf("%d", tag->load[i]);
    }

    if (colours)
        printf(_PUNCT "]");
    else
        printf("]");
}

void print_tag_long_array(Tag_t *ptr)
{
    const Tag_long_array_t *tag = (Tag_long_array_t *) ptr;
    if (colours)
        printf(_PUNCT "[" _TYPE "L" _PUNCT ";");
    else
        printf("[L;");
    space();

    for (int i = 0; i < tag->length; i++) {
        if (i > 0) {
            if (colours)
                printf(_PUNCT ",");
            else
                printf(",");
            space();
        }

        if (colours)
            printf(_VAL "%ld" _TYPE "l", tag->load[i]);
        else
            printf("%ldl", tag->load[i]);
    }

    if (colours)
        printf(_PUNCT "]");
    else
        printf("]");
}

static uint8_t is_safe_str(Tag_string_t const *tag)
{
    for (int i = 0; i < tag->length; i++) {
        uint8_t c = tag->load[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' ||
               c == '.' || c == '+'))
        {
            return 0;
        }
    }
    return 1;
}

void print_named_tag(Named_tag_t *tag)
{
    uint8_t safe = is_safe_str(tag->name);

    if (colours)
        printf(_STR "\"");
    else if (!safe)
        printf("\"");

    print_safe_str(tag->name);

    if (colours)
        printf("\"" _PUNCT ":");
    else if (!safe)
        printf("\":");
    else
        printf(":");
    space();

    print_functions[tag->type](tag->tag);
}

void print_nbt_tag(Named_tag_t *tag)
{
    if (tag->type != TAG_Compound) {
        fprintf(stderr, _ERR "Error! Root tag is not compound." _CLEAR);
        exit(1);
    }

    print_tag_compound(tag->tag);
}

inline void indent_line()
{
    if (colours)
        for (int i = 0; i < _indent; ++i) printf("  ");
}

inline void new_line()
{
    if (colours) printf("\n");
}

inline void space()
{
    if (colours) printf(" ");
}

inline void increase_indentation()
{
    ++_indent;
}

inline void decrease_indentation()
{
    --_indent;
}