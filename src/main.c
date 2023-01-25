#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <unistd.h>

#include <ast.h>
#include <compress.h>
#include <decompress.h>
#include <parse.h>
#include <print.h>

int main(int argc, const char **argv)
{
    uint8_t parse = 0, compr = 0;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-p"))
            parse = 1;
        else if (!strcmp(argv[i], "-c"))
            compr = 1;
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf(
                "Usage: %s [options] < input_file > output_file\n"
                "\n"
                "Reads NBT file and outputs NBT file. The default behaviour "
                "is to read binary NBT and output text NBT. Input and output "
                "are stdin and stdout, respectively, so you must use pipes "
                "and redirects. If the output is associated with a terminal, "
                "the program automatically prints the output in colour.\n"
                "\n"
                "  -p : Parses input as text NBT.\n"
                "  -c : Compresses output as binary NBT.\n"
                "\n", argv[0]
            );
            return 0;
        }
    }

    Named_tag_t *tag;

    if (isatty(fileno(stdout)))
        colours = 1;

    if (parse)
        tag = parse_nbt_tag();
    else
        tag = nbt_decompress();

    if (!tag) {
        return -1;
    }

    if (compr) {
        fprintf(stderr, _OK "Compressing data...\n" _CLEAR);
        nbt_compress(tag);
        if (colours)
            printf("\n");
    }
    else {
        print_nbt_tag(tag);
        if (colours)
            printf(_CLEAR "\n");
        else
            printf("\n");
    }

    free_nbt_tag(tag);

    return 0;
}