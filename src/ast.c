#include <ast.h>
#include <print.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void (*free_functions[])(Tag_t *) = {
    free_tag_end,        free_tag_byte,  free_tag_short,    free_tag_int,
    free_tag_long,       free_tag_float, free_tag_double,   free_tag_byte_array,
    free_tag_string,     free_tag_list,  free_tag_compound, free_tag_int_array,
    free_tag_long_array,
};

Tag_t *new_end()
{
    Tag_t *new = (Tag_t *) malloc(sizeof(Tag_t));
    new->type = TAG_End;
    return new;
}

void free_tag_end(Tag_t *ptr)
{
    free(ptr);
}

Tag_byte_t *new_byte(int8_t load)
{
    Tag_byte_t *new = (Tag_byte_t *) malloc(sizeof(Tag_byte_t));
    new->load = load;
    new->type = TAG_Byte;
    return new;
}

void free_tag_byte(Tag_t *ptr)
{
    Tag_byte_t *tag = (Tag_byte_t *) ptr;
    free(tag);
}

Tag_short_t *new_short(int16_t load)
{
    Tag_short_t *new = (Tag_short_t *) malloc(sizeof(Tag_short_t));
    new->load = load;
    new->type = TAG_Short;
    return new;
}

void free_tag_short(Tag_t *ptr)
{
    Tag_short_t *tag = (Tag_short_t *) ptr;
    free(tag);
}

Tag_int_t *new_int(int32_t load)
{
    Tag_int_t *new = (Tag_int_t *) malloc(sizeof(Tag_int_t));
    new->load = load;
    new->type = TAG_Int;
    return new;
}

void free_tag_int(Tag_t *ptr)
{
    Tag_int_t *tag = (Tag_int_t *) ptr;
    free(tag);
}

Tag_long_t *new_long(int64_t load)
{
    Tag_long_t *new = (Tag_long_t *) malloc(sizeof(Tag_long_t));
    new->load = load;
    new->type = TAG_Long;
    return new;
}

void free_tag_long(Tag_t *ptr)
{
    Tag_long_t *tag = (Tag_long_t *) ptr;
    free(tag);
}

Tag_float_t *new_float(float load)
{
    Tag_float_t *new = (Tag_float_t *) malloc(sizeof(Tag_float_t));
    new->load = load;
    new->type = TAG_Float;
    return new;
}

void free_tag_float(Tag_t *ptr)
{
    Tag_float_t *tag = (Tag_float_t *) ptr;
    free(tag);
}

Tag_double_t *new_double(double load)
{
    Tag_double_t *new = (Tag_double_t *) malloc(sizeof(Tag_double_t));
    new->load = load;
    new->type = TAG_Double;
    return new;
}

void free_tag_double(Tag_t *ptr)
{
    Tag_double_t *tag = (Tag_double_t *) ptr;
    free(tag);
}

Tag_byte_array_t *new_byte_array(int32_t length)
{
    Tag_byte_array_t *new =
        (Tag_byte_array_t *) malloc(sizeof(Tag_byte_array_t));
    new->length = length;
    new->load = (int8_t *) malloc(length);
    new->type = TAG_Byte_Array;
    return new;
}

void free_tag_byte_array(Tag_t *ptr)
{
    Tag_byte_array_t *tag = (Tag_byte_array_t *) ptr;
    free(tag->load);
    free(tag);
}

Tag_string_t *new_string(int16_t length)
{
    Tag_string_t *new = (Tag_string_t *) malloc(sizeof(Tag_string_t));
    new->length = length;
    new->load = (int8_t *) malloc(length + 1);
    new->load[length] = 0x00;
    new->type = TAG_String;
    return new;
}

void free_tag_string(Tag_t *ptr)
{
    Tag_string_t *tag = (Tag_string_t *) ptr;
    free(tag->load);
    free(tag);
}

Tag_list_t *new_list(int8_t type, int32_t length)
{
    Tag_list_t *new = (Tag_list_t *) malloc(sizeof(Tag_list_t));
    new->list_type = type;
    new->length = length;
    new->load = (Tag_t **) malloc(length * sizeof(Tag_t *));
    new->type = TAG_List;
    return new;
}

void free_tag_list(Tag_t *ptr)
{
    Tag_list_t *tag = (Tag_list_t *) ptr;
    for (int i = 0; i < tag->length; i++)
        free_functions[tag->list_type](tag->load[i]);
    free(tag->load);
    free(tag);
}

Tag_compound_t *new_compound(Compound_node_t *list)
{
    Tag_compound_t *new = (Tag_compound_t *) malloc(sizeof(Tag_compound_t));
    new->load = finalise_compound_list(list);
    new->type = TAG_Compound;
    return new;
}

void free_tag_compound(Tag_t *ptr)
{
    Tag_compound_t *tag = (Tag_compound_t *) ptr;
    for (int i = 0; tag->load[i]; i++) free_named_tag(tag->load[i]);
    free(tag->load);
    free(tag);
}

Tag_int_array_t *new_int_array(int32_t length)
{
    Tag_int_array_t *new = (Tag_int_array_t *) malloc(sizeof(Tag_int_array_t));
    new->length = length;
    new->load = (int32_t *) malloc(length * sizeof(int32_t));
    new->type = TAG_Int_Array;
    return new;
}

void free_tag_int_array(Tag_t *ptr)
{
    Tag_int_array_t *tag = (Tag_int_array_t *) ptr;
    free(tag->load);
    free(tag);
}

Tag_long_array_t *new_long_array(int32_t length)
{
    Tag_long_array_t *new =
        (Tag_long_array_t *) malloc(sizeof(Tag_long_array_t));
    new->length = length;
    new->load = (int64_t *) malloc(length * sizeof(int64_t));
    new->type = TAG_Long_Array;
    return new;
}

void free_tag_long_array(Tag_t *ptr)
{
    Tag_long_array_t *tag = (Tag_long_array_t *) ptr;
    free(tag->load);
    free(tag);
}

Named_tag_t *new_named_tag(int8_t type, Tag_string_t *name, Tag_t *tag)
{
    Named_tag_t *new = (Named_tag_t *) malloc(sizeof(Named_tag_t));
    new->type = type;
    new->name = name;
    new->tag = tag;
    return new;
}

void free_named_tag(Named_tag_t *tag)
{
    free_tag_string((Tag_t *) tag->name);
    free_functions[tag->type]((Tag_t *) tag->tag);
    free(tag);
}

void free_nbt_tag(Named_tag_t *tag)
{
    free_tag_string((Tag_t *) tag->name);
    free_tag_compound((Tag_t *) tag->tag);
    free(tag);
}

inline Compound_node_t *new_compound_list()
{
    return NULL;
}

Compound_node_t *add_compound_node(Compound_node_t *list, Named_tag_t *tag)
{
    Compound_node_t *new = (Compound_node_t *) malloc(sizeof(Compound_node_t));
    new->tag = tag;
    new->previous = list;
    return new;
}

Named_tag_t **finalise_compound_list(Compound_node_t *list)
{
    Compound_node_t *ptr;
    int i, list_size = 1;

    for (ptr = list; ptr; ptr = ptr->previous) list_size++;
    Named_tag_t **array =
        (Named_tag_t **) malloc(list_size * sizeof(Named_tag_t *));

    ptr = list;
    array[list_size - 1] = NULL;
    i = list_size - 2;

    while (ptr) {
        Compound_node_t *ref = ptr;
        array[i] = (Named_tag_t *) ptr->tag;
        ptr = ptr->previous;
        free(ref);
        i--;
    }
    return array;
}

inline List_node_t *new_nodes_list()
{
    return NULL;
}

List_node_t *add_list_node(List_node_t *list, Tag_t *tag)
{
    List_node_t *new = (List_node_t *) malloc(sizeof(List_node_t));
    new->tag = tag;
    new->previous = list;
    return new;
}

Tag_list_t *finalise_nodes_list(int8_t type, List_node_t *list)
{
    List_node_t *ptr;
    int i, list_size = 0;
    for (ptr = list; ptr; ptr = ptr->previous) {
        list_size++;
    }
    Tag_t **array = (Tag_t **) malloc(list_size * sizeof(Tag_t *));

    ptr = list;
    i = list_size - 1;

    while (ptr) {
        List_node_t *ref = ptr;
        array[i] = (Tag_t *) ptr->tag;
        ptr = ptr->previous;
        free(ref);
        i--;
    }

    Tag_list_t *tag = (Tag_list_t *) malloc(sizeof(Tag_list_t));
    tag->list_type = type;
    tag->length = list_size;
    tag->load = array;
    tag->type = TAG_List;
    return tag;
}
