#pragma once

#include <ast.h>
#include <stdint.h>

//// MACROS ////

#define CHUNK       0x1000
#define windowBits  15
#define ENABLE_GZIP 16

//// DECLARATIONS ////

Named_tag_t *nbt_decompress();

Named_tag_t *read_nbt_tag();
Named_tag_t *read_TAG();

Tag_t *read_TAG_End();
Tag_t *read_TAG_Byte();
Tag_t *read_TAG_Short();
Tag_t *read_TAG_Int();
Tag_t *read_TAG_Long();
Tag_t *read_TAG_Float();
Tag_t *read_TAG_Double();
Tag_t *read_TAG_Byte_Array();
Tag_t *read_TAG_String();
Tag_t *read_TAG_List();
Tag_t *read_TAG_Compound();
Tag_t *read_TAG_Int_Array();
Tag_t *read_TAG_Long_Array();