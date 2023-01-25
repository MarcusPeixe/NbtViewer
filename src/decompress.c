#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include <ast.h>
#include <decompress.h>
#include <print.h>

//// VARIABLES ////

static int buf_index = 0;
static int buf_len = 0;

static uint8_t in_buf[CHUNK];
static uint8_t *out_buf;

static Tag_t *(*function_table[])() = {
    read_TAG_End,        read_TAG_Byte,  read_TAG_Short,    read_TAG_Int,
    read_TAG_Long,       read_TAG_Float, read_TAG_Double,   read_TAG_Byte_Array,
    read_TAG_String,     read_TAG_List,  read_TAG_Compound, read_TAG_Int_Array,
    read_TAG_Long_Array,
};

//// DECLARATIONS ////

static uint8_t next();

static void read_8b(void *ptr);
static void read_16b(void *ptr);
static void read_32b(void *ptr);
static void read_64b(void *ptr);

//// DEFINITIONS ////

Named_tag_t *nbt_decompress()
{
    z_stream strm = {0};
    z_streamp strmp = &strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in_buf;
    strm.avail_in = 0;

    out_buf = NULL;

    if (inflateInit2(strmp, windowBits | ENABLE_GZIP)) {
        fprintf(stderr, _ERR "Error!\n" _CLEAR);
        return NULL;
    }

    while (1) {
        int bytes_read = fread(in_buf, 1, CHUNK, stdin);
        strm.avail_in = bytes_read;
        strm.next_in = in_buf;

        fprintf(stderr, _OK "New chunk of compressed data\n");

        do {
            strm.avail_out = CHUNK;
            out_buf = realloc(out_buf, strm.total_out + CHUNK);
            strm.next_out = out_buf + strm.total_out;

            fprintf(stderr, _OK "Output buffer now has %ld bytes.\n",
                    strm.total_out + CHUNK);

            int status = inflate(strmp, Z_NO_FLUSH);

            switch (status) {
            case Z_OK:
            case Z_STREAM_END:
            case Z_BUF_ERROR:
                break;

            default:
                inflateEnd(strmp);
                fprintf(stderr, _ERR "Gzip error %d.\n" _CLEAR, status);
                return NULL;
            }

        } while (strm.avail_out == 0);

        if (feof(stdin)) {
            inflateEnd(strmp);
            break;
        }
    }

    fprintf(stderr,
            _CLEAR _OK "Decompressed successfully, %ld bytes.\n" _CLEAR,
            strm.total_out);

    buf_index = 0;
    buf_len = strm.total_out;
    Named_tag_t *tag = read_nbt_tag();

    free(out_buf);
    return tag;
}

Named_tag_t *read_nbt_tag()
{
    enum TAG_TYPE type = (enum TAG_TYPE) next();
    Tag_string_t *name = (Tag_string_t *) read_TAG_String();

    if (type != TAG_Compound) {
        fprintf(stderr, _ERR "Error! Root tag is not compound." _CLEAR);
        return NULL;
    }
    return new_named_tag(type, name, function_table[type]());
}

Named_tag_t *read_TAG()
{
    enum TAG_TYPE type = (enum TAG_TYPE) next();
    if (type == TAG_End) return NULL;

    Tag_string_t *name = (Tag_string_t *) read_TAG_String();

    return new_named_tag(type, name, function_table[type]());
}

Tag_t *read_TAG_End()
{
    next();
    return new_end();
}

Tag_t *read_TAG_Byte()
{
    int8_t n;
    read_8b(&n);
    return (Tag_t *) new_byte(n);
}

Tag_t *read_TAG_Short()
{
    int16_t n;
    read_16b(&n);
    return (Tag_t *) new_short(n);
}

Tag_t *read_TAG_Int()
{
    int32_t n;
    read_32b(&n);
    return (Tag_t *) new_int(n);
}

Tag_t *read_TAG_Long()
{
    int64_t n;
    read_64b(&n);
    return (Tag_t *) new_long(n);
}

Tag_t *read_TAG_Float()
{
    float n;
    read_32b(&n);
    return (Tag_t *) new_float(n);
}

Tag_t *read_TAG_Double()
{
    double n;
    read_64b(&n);
    return (Tag_t *) new_double(n);
}

Tag_t *read_TAG_Byte_Array()
{
    int32_t length;
    read_32b(&length);

    Tag_byte_array_t *tag = new_byte_array(length);

    for (int i = 0; i < length; i++) {
        int8_t n;
        read_8b(&n);

        tag->load[i] = n;
    }

    return (Tag_t *) tag;
}

Tag_t *read_TAG_String()
{
    int16_t length;
    read_16b(&length);

    Tag_string_t *tag = new_string(length);

    for (int i = 0; i < length; i++) {
        read_8b(tag->load + i);
    }

    return (Tag_t *) tag;
}

Tag_t *read_TAG_List()
{
    enum TAG_TYPE type = (enum TAG_TYPE) next();

    int32_t length;
    read_32b(&length);

    Tag_list_t *tag = new_list(type, length);

    for (int i = 0; i < length; i++) {
        tag->load[i] = function_table[type]();
    }

    return (Tag_t *) tag;
}

Tag_t *read_TAG_Compound()
{
    int length = 0;
    Compound_node_t *tag_list = new_compound_list();

    while (1) {
        Named_tag_t *tag = read_TAG();
        if (tag) {
            tag_list = add_compound_node(tag_list, tag);
            length++;
        }
        else
            break;
    }

    return (Tag_t *) new_compound(tag_list);
}

Tag_t *read_TAG_Int_Array()
{
    int32_t length;
    read_32b(&length);

    Tag_int_array_t *tag = new_int_array(length);

    for (int i = 0; i < length; i++) {
        int32_t n;
        read_32b(&n);

        tag->load[i] = n;
    }

    return (Tag_t *) tag;
}

Tag_t *read_TAG_Long_Array()
{
    int32_t length;
    read_32b(&length);

    Tag_long_array_t *tag = new_long_array(length);

    for (int i = 0; i < length; i++) {
        int64_t n;
        read_64b(&n);

        tag->load[i] = n;
    }

    return (Tag_t *) tag;
}

static void read_8b(void *ptr)
{
    uint8_t r = (uint8_t) next();
    *(uint8_t *) ptr = r;
}

static void read_16b(void *ptr)
{
    uint16_t r = (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    *(uint16_t *) ptr = r;
}

static void read_32b(void *ptr)
{
    uint32_t r = (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    *(uint32_t *) ptr = r;
}

static void read_64b(void *ptr)
{
    uint64_t r = (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    r = r << 8 | (uint8_t) next();
    *(uint64_t *) ptr = r;
}

static uint8_t next()
{
    if (buf_index >= buf_len) {
        fprintf(stderr, _ERR "ERROR! Unexpected EOF.\n" _CLEAR);
        exit(1);
    }
    return out_buf[buf_index++];
}
