#pragma once

#include <ast.h>
#include <stdint.h>

//// MACROS ////

#define CHUNK       0x1000
#define windowBits  15
#define ENABLE_GZIP 16

//// DECLARATIONS ////

void nbt_compress(Named_tag_t *);

void write_nbt_tag(Named_tag_t *);
void write_TAG(Named_tag_t *);

void write_TAG_End(Tag_t *);
void write_TAG_Byte(Tag_t *);
void write_TAG_Short(Tag_t *);
void write_TAG_Int(Tag_t *);
void write_TAG_Long(Tag_t *);
void write_TAG_Float(Tag_t *);
void write_TAG_Double(Tag_t *);
void write_TAG_Byte_Array(Tag_t *);
void write_TAG_String(Tag_t *);
void write_TAG_List(Tag_t *);
void write_TAG_Compound(Tag_t *);
void write_TAG_Int_Array(Tag_t *);
void write_TAG_Long_Array(Tag_t *);