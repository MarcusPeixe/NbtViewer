#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include <ast.h>
#include <compress.h>
#include <print.h>

//// VARIABLES ////

static int buf_index = 0;
static int buf_len = 0;

static uint8_t *in_buf;
static uint8_t *out_buf;

static void (*function_table[])(Tag_t *) = {
    write_TAG_End,        write_TAG_Byte,       write_TAG_Short,
    write_TAG_Int,        write_TAG_Long,       write_TAG_Float,
    write_TAG_Double,     write_TAG_Byte_Array, write_TAG_String,
    write_TAG_List,       write_TAG_Compound,   write_TAG_Int_Array,
    write_TAG_Long_Array,
};

//// DECLARATIONS ////

static void next(uint8_t c);

static void write_8b(void *ptr);
static void write_16b(void *ptr);
static void write_32b(void *ptr);
static void write_64b(void *ptr);

//// DEFINITIONS ////

void nbt_compress(Named_tag_t *tag)
{
    z_stream strm = {0};
    z_streamp strmp = &strm;

    fprintf(stderr, _OK "Write to buffer was successful, %d bytes.\n" _CLEAR,
            buf_index);
    write_nbt_tag(tag);

    int input_length = buf_index;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in_buf;
    strm.avail_in = 0;

    out_buf = NULL;

    if (deflateInit2(strmp, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     windowBits | ENABLE_GZIP, 8, Z_DEFAULT_STRATEGY))
    {
        fprintf(stderr, _ERR "Error!\n" _CLEAR);
        return;
    }

    // if (inflateInit2(strmp, windowBits | ENABLE_GZIP)) {
    //     fprintf(stderr, _ERR "Error!\n" _CLEAR);
    //     return NULL;
    // }

    while (1) {
        strm.avail_in = input_length;
        strm.next_in = in_buf;

        // fprintf(stderr, _OK "New chunk of uncompressed data\n");

        do {
            strm.avail_out = input_length;
            out_buf = realloc(out_buf, strm.total_out + input_length);
            strm.next_out = out_buf + strm.total_out;

            fprintf(stderr, _OK "Output buffer now has %ld bytes.\n",
                    strm.total_out + input_length);

            int status = deflate(strmp, Z_FINISH);

            switch (status) {
            case Z_OK:
            case Z_STREAM_END:
            case Z_BUF_ERROR:
                break;

            default:
                deflateEnd(strmp);
                fprintf(stderr, _ERR "Gzip error %d.\n" _CLEAR, status);
                return;
            }

        } while (strm.avail_out == 0);

        if (strm.total_in >= input_length) {
            deflateEnd(strmp);
            break;
        }
    }

    fprintf(stderr,
            _CLEAR _OK "Compressed successfully, %ld bytes.\n" _CLEAR,
            strm.total_out);

    fwrite(out_buf, strm.total_out, 1, stdout);

    free(in_buf);
    free(out_buf);
}

void write_nbt_tag(Named_tag_t *ptr)
{
    if (ptr->type != TAG_Compound) {
        fprintf(stderr, _ERR "Error! Root tag is not compound." _CLEAR);
        return;
    }
    write_8b(&ptr->type);

    write_TAG_String((Tag_t *) ptr->name);

    function_table[ptr->type](ptr->tag);
}

void write_TAG(Named_tag_t *ptr)
{
    write_8b(&ptr->type);

    write_TAG_String((Tag_t *) ptr->name);

    function_table[ptr->type](ptr->tag);
}

void write_TAG_End(Tag_t *ptr)
{
    next(0);
}

void write_TAG_Byte(Tag_t *ptr)
{
    Tag_byte_t *tag = (Tag_byte_t *) ptr;
    write_8b(&tag->load);
}

void write_TAG_Short(Tag_t *ptr)
{
    Tag_short_t *tag = (Tag_short_t *) ptr;
    write_16b(&tag->load);
}

void write_TAG_Int(Tag_t *ptr)
{
    Tag_int_t *tag = (Tag_int_t *) ptr;
    write_32b(&tag->load);
}

void write_TAG_Long(Tag_t *ptr)
{
    Tag_long_t *tag = (Tag_long_t *) ptr;
    write_64b(&tag->load);
}

void write_TAG_Float(Tag_t *ptr)
{
    Tag_float_t *tag = (Tag_float_t *) ptr;
    write_32b(&tag->load);
}

void write_TAG_Double(Tag_t *ptr)
{
    Tag_double_t *tag = (Tag_double_t *) ptr;
    write_64b(&tag->load);
}

void write_TAG_Byte_Array(Tag_t *ptr)
{
    Tag_byte_array_t *tag = (Tag_byte_array_t *) ptr;
    write_32b(&tag->length);

    for (int i = 0; i < tag->length; i++) {
        write_8b(tag->load + i);
    }
}

void write_TAG_String(Tag_t *ptr)
{
    Tag_string_t *tag = (Tag_string_t *) ptr;
    write_16b(&tag->length);

    for (int i = 0; i < tag->length; i++) {
        write_8b(tag->load + i);
    }
}

void write_TAG_List(Tag_t *ptr)
{
    Tag_list_t *tag = (Tag_list_t *) ptr;
    write_8b(&tag->list_type);
    write_32b(&tag->length);

    for (int i = 0; i < tag->length; i++) {
        function_table[tag->list_type](tag->load[i]);
    }
}

void write_TAG_Compound(Tag_t *ptr)
{
    Tag_compound_t *tag = (Tag_compound_t *) ptr;

    for (int i = 0; tag->load[i]; i++) {
        write_TAG(tag->load[i]);
    }

    write_TAG_End(NULL);
}

void write_TAG_Int_Array(Tag_t *ptr)
{
    Tag_int_array_t *tag = (Tag_int_array_t *) ptr;
    write_32b(&tag->length);

    for (int i = 0; i < tag->length; i++) {
        write_32b(tag->load + i);
    }
}

void write_TAG_Long_Array(Tag_t *ptr)
{
    Tag_long_array_t *tag = (Tag_long_array_t *) ptr;
    write_32b(&tag->length);

    for (int i = 0; i < tag->length; i++) {
        write_64b(tag->load + i);
    }
}

static void write_8b(void *ptr)
{
    uint8_t r = *(uint8_t *) ptr;
    next(r);
}

static void write_16b(void *ptr)
{
    uint16_t r = *(uint16_t *) ptr;
    next(r >> 8);
    next(r);
}

static void write_32b(void *ptr)
{
    uint32_t r = *(uint32_t *) ptr;
    next(r >> 24);
    next(r >> 16);
    next(r >> 8);
    next(r);
}

static void write_64b(void *ptr)
{
    uint64_t r = *(uint64_t *) ptr;
    next(r >> 56);
    next(r >> 48);
    next(r >> 40);
    next(r >> 32);
    next(r >> 24);
    next(r >> 16);
    next(r >> 8);
    next(r);
}

static void next(uint8_t c)
{
    if (buf_index >= buf_len) {
        buf_len += CHUNK;
        in_buf = realloc(in_buf, buf_len);
    }
    in_buf[buf_index++] = c;
}
