#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include <ast.h>
#include <parse.h>
#include <print.h>

//// VARIABLES ////

static error_t *global_error = NULL;

static int buf_index = 0;
static int buf_len = 0;

static char *out_buf = NULL;

static Tag_t *(*function_table[])() = {
    NULL,
    parse_TAG_Byte,
    parse_TAG_Short,
    parse_TAG_Int,
    parse_TAG_Long,
    parse_TAG_Float,
    parse_TAG_Double,
    parse_TAG_Byte_Array,
    parse_TAG_String,
    parse_TAG_List,
    parse_TAG_Compound,
    parse_TAG_Int_Array,
    parse_TAG_Long_Array,
};

//// DECLARATIONS ////

static void free_error(error_t *error);
static void clear_error();

static uint8_t next();
static uint8_t cmp_next(const char *str);
static uint8_t seek();
static void skip_whitespace();

static int get_state();
static void set_state(int);

static void parser_init();
static void parser_end();

//// DEFINITIONS ////

Named_tag_t *parse_nbt_tag()
{
    parser_init();

    skip_whitespace();
    Tag_t *tag = parse_TAG_Compound();
    if (tag) {
        fprintf(stderr, _OK "Parsed successfully!\n\n" _CLEAR);
        free_error(global_error);
        return new_named_tag(TAG_Compound, new_string(0), tag);
    }
    else {
        fprintf(stderr, "\n");
        print_error(global_error);
        free_error(global_error);
        return NULL;
    }
    parser_end();
}

Tag_t *parse_any_data()
{
    static Tag_t *(*parse_funcs_ordered[])() = {
        // Integer types
        parse_TAG_Int,
        parse_TAG_Byte,
        parse_TAG_Short,
        parse_TAG_Long,
        // Floating point types
        parse_TAG_Double,
        parse_TAG_Float,
        // Array types
        parse_TAG_Byte_Array,
        parse_TAG_Int_Array,
        parse_TAG_Long_Array,
        // Aggregate types
        parse_TAG_List,
        parse_TAG_String,
        parse_TAG_Compound,
    };

    const static char *error_types[] = {
        "Failed to parse TAG_Int",        "Failed to parse TAG_Byte",
        "Failed to parse TAG_Short",      "Failed to parse TAG_Long",
        "Failed to parse TAG_Double",     "Failed to parse TAG_Float",
        "Failed to parse TAG_Byte_Array", "Failed to parse TAG_Int_Array",
        "Failed to parse TAG_Long_Array", "Failed to parse TAG_List",
        "Failed to parse TAG_String",     "Failed to parse TAG_Compound",
    };

    skip_whitespace();
    int state = get_state();
    Tag_t *longest = NULL;
    int longest_state;

    error_t *relevant = NULL;
    int relevant_location;
    int relevant_type;
    for (int i = 0; i < TOTAL_TYPES - 1; i++) {
        set_state(state);

        Tag_t *this = parse_funcs_ordered[i]();
        int this_state = get_state();
        if (this) {
            if (!longest) {
                longest = this;
                longest_state = this_state;
            }
            else if (longest_state < this_state) {
                free_functions[longest->type](longest);
                longest = this;
                longest_state = this_state;
            }
            else {
                free_functions[this->type](this);
            }
        }
        else {
            if (!relevant) {
                relevant = global_error;
                relevant_location = relevant->location;
                relevant_type = i;
                global_error = NULL;
            }
            else if (relevant_location < global_error->location) {
                free_error(relevant);

                relevant = global_error;
                relevant_location = relevant->location;
                relevant_type = i;
                global_error = NULL;
            }
            else {
                clear_error();
            }
        }
    }
    if (longest) {
        set_state(longest_state);
        clear_error();
        return longest;
    }
    else if (relevant) {
        if (relevant_location > state) {
            global_error = relevant;
            append_error(get_state(), error_types[relevant_type]);
        }
        else {
            free_error(relevant);
            raise_error(get_state(), "Couldn't parse tag value.");
        }
    }
    set_state(state);
    return NULL;
}

Tag_string_t *parse_tag_name()
{
    int state = get_state();
    int16_t length = 0;
    char *buf = NULL;

    if (seek() == '\'' || seek() == '"') {
        return (Tag_string_t *) parse_TAG_String();
    }

    int i = 0;
    while (1) {
        if (i >= length) {
            length += CHUNK;
            buf = realloc(buf, length);
        }
        char c = seek();

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-' ||
             c == '.' || c == '+')
        {
            buf[i++] = next();
        }
        else {
            break;
        }
    }
    if (i == 0) {
        raise_error(state, "Expected tag name.");
        free(buf);
        return NULL;
    }
    Tag_string_t *tag = new_string(i);
    memcpy(tag->load, buf, i);
    free(buf);

    return tag;
}

Named_tag_t *parse_named_tag()
{
    int state = get_state();

    Tag_string_t *name = parse_tag_name();
    // Tag_string_t *name = (Tag_string_t *) parse_TAG_String();
    if (!name) {
        append_error(state, "Invalid tag.");
        set_state(state);
        return NULL;
    }
    skip_whitespace();
    if (seek() == ':') {
        next();
    }
    else {
        raise_error(get_state(), "Expected a colon.");
        free_tag_string((Tag_t *) name);
        set_state(state);
        return NULL;
    }
    skip_whitespace();
    Tag_t *tag = parse_any_data();
    if (!tag) {
        append_error(state, "Invalid tag.");
        free_tag_string((Tag_t *) name);
        set_state(state);
        return NULL;
    }

    return new_named_tag(tag->type, name, tag);
}

Tag_t *parse_TAG_Byte()
{
    int state = get_state();
    uint8_t read[8];

    if (cmp_next("true")) {
        return (Tag_t *) new_byte(1);
    }
    else if (cmp_next("false")) {
        return (Tag_t *) new_byte(0);
    }
    else if (seek() == '-')
        next();
    while (1) {
        char c = seek();
        if (c < '0' || c > '9') break;
        next();
    }
    if (seek() == 'b' || seek() == 'B') next();

    if (state == get_state()) {
        raise_error(state, "No digits found.");
        set_state(state);
        return NULL;
    }
    if (!sscanf(out_buf + state, " %ld", (long *) read)) {
        raise_error(get_state(), "Not a valid byte.");
        set_state(state);
        return NULL;
    }
    return (Tag_t *) new_byte(*(long *) read);
}

Tag_t *parse_TAG_Short()
{
    int state = get_state();
    uint8_t read[8];

    if (seek() == '-') next();
    while (1) {
        char c = seek();
        if (c < '0' || c > '9') break;
        next();
    }
    if (seek() == 's' || seek() == 'S') next();

    if (state == get_state()) {
        raise_error(state, "No digits found.");
        set_state(state);
        return NULL;
    }
    if (!sscanf(out_buf + state, " %ld", (long *) read)) {
        raise_error(get_state(), "Not a valid short.");
        set_state(state);
        return NULL;
    }
    return (Tag_t *) new_short(*(long *) read);
}

Tag_t *parse_TAG_Int()
{
    int state = get_state();
    uint8_t read[8];

    if (seek() == '-') next();
    while (1) {
        char c = seek();
        if (c < '0' || c > '9') break;
        next();
    }

    if (state == get_state()) {
        raise_error(state, "No digits found.");
        set_state(state);
        return NULL;
    }
    if (!sscanf(out_buf + state, " %ld", (long *) read)) {
        raise_error(get_state(), "Not a valid int.");
        set_state(state);
        return NULL;
    }
    Tag_int_t *tag = new_int(*(long *) read);
    return (Tag_t *) tag;
}

Tag_t *parse_TAG_Long()
{
    int state = get_state();
    uint8_t read[8];

    if (seek() == '-') next();
    while (1) {
        char c = seek();
        if (c < '0' || c > '9') break;
        next();
    }
    if (seek() == 'l' || seek() == 'L') next();

    if (state == get_state()) {
        raise_error(state, "No digits found.");
        set_state(state);
        return NULL;
    }
    if (!sscanf(out_buf + state, " %ld", (long *) read)) {
        raise_error(get_state(), "Not a valid long.");
        set_state(state);
        return NULL;
    }
    return (Tag_t *) new_long(*(long *) read);
}

Tag_t *parse_TAG_Float()
{
    int state = get_state();
    uint8_t read[8];

    if (seek() == '-') next();
    while (1) {
        char c = seek();
        if ((c < '0' || c > '9') && c != '.' && c != 'e') break;
        next();
    }
    if (seek() == 'f' || seek() == 'F') next();

    if (state == get_state()) {
        raise_error(state, "No digits found.");
        set_state(state);
        return NULL;
    }
    if (!sscanf(out_buf + state, " %lg", (double *) read)) {
        raise_error(get_state(), "Not a valid float.");
        set_state(state);
        return NULL;
    }
    return (Tag_t *) new_float(*(double *) read);
}

Tag_t *parse_TAG_Double()
{
    int state = get_state();
    uint8_t read[8];

    if (seek() == '-') next();
    while (1) {
        char c = seek();
        if ((c < '0' || c > '9') && c != '.' && c != 'e') break;
        next();
    }
    if (seek() == 'd' || seek() == 'D') next();

    if (state == get_state()) {
        raise_error(state, "No digits found.");
        set_state(state);
        return NULL;
    }
    if (!sscanf(out_buf + state, " %lf", (double *) read)) {
        raise_error(get_state(), "Not a valid double.");
        set_state(state);
        return NULL;
    }
    return (Tag_t *) new_double(*(double *) read);
}

Tag_t *parse_TAG_Byte_Array()
{
    int state = get_state();
    int32_t length = 0;
    int8_t *buf = NULL;

    if (!cmp_next("[B;")) {
        raise_error(state, "Invalid byte array.");
        return NULL;
    }
    int i = 0;
    skip_whitespace();
    if (seek() == ']') {
        next();
        Tag_byte_array_t *tag = new_byte_array(i);
        free(buf);
        return (Tag_t *) tag;
    }
    while (1) {
        if (i >= length) {
            length += CHUNK;
            buf = realloc(buf, length);
        }

        int local_state = get_state();
        uint8_t read[8];

        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (cmp_next("true")) {
            buf[i] = 1;
        }
        else if (cmp_next("false")) {
            buf[i] = 0;
        }
        else {
            if (seek() == '-') next();
            while (1) {
                char c = seek();
                if (c < '0' || c > '9') break;
                next();
            }
            if (seek() == 'b' || seek() == 'B') next();

            if (local_state == get_state()) {
                raise_error(local_state, "No digits found.");
                set_state(state);
                free(buf);
                return NULL;
            }
            if (!sscanf(out_buf + local_state, " %ld", (long *) read)) {
                raise_error(get_state(), "Not a valid byte.");
                set_state(state);
                free(buf);
                return NULL;
            }
            buf[i] = *(long *) read;
        }

        i++;
        int comma_state = get_state();
        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (seek() == ',') {
            next();
        }
        else {
            raise_error(comma_state, "Expected a comma or closing brackets.");
            set_state(state);
            free(buf);
            return NULL;
        }
    }

    Tag_byte_array_t *tag = new_byte_array(i);
    memcpy(tag->load, buf, i);
    free(buf);
    return (Tag_t *) tag;
}

Tag_t *parse_TAG_String()
{
    int state = get_state();
    int16_t length = 0;
    char *buf = NULL;

    char delim;

    if (seek() == '"' || seek() == '\'')
        delim = next();
    else {
        raise_error(state, "Invalid string.");
        set_state(state);
        return NULL;
    }

    int i = 0;
    while (1) {
        if (i >= length) {
            length += CHUNK;
            buf = realloc(buf, length);
        }

        if (seek() == delim) {
            next();
            break;
        }
        else if (seek() == '\n') {
            raise_error(get_state(), "Multiline string literal.");
            set_state(state);
            free(buf);
            return NULL;
        }
        else if (seek() == '\\') {
            next();
            switch (seek()) {
            case 'a':
                buf[i++] = 0x07; next(); break;
            case 'b':
                buf[i++] = 0x08; next(); break;
            case 'e':
                buf[i++] = 0x1B; next(); break;
            case 'f':
                buf[i++] = 0x0C; next(); break;
            case 'n':
                buf[i++] = 0x0A; next(); break;
            case 'r':
                buf[i++] = 0x0D; next(); break;
            case 't':
                buf[i++] = 0x09; next(); break;
            case 'v':
                buf[i++] = 0x0B; next(); break;
            case '\\':
                buf[i++] = '\\'; next(); break;
            case '"':
                buf[i++] = '"';  next(); break;
            case '\'':
                buf[i++] = '\''; next(); break;
            case 'x': {
                unsigned int value;
                char num_buf[3] = {0};
                for (int i = 0; i < 2; i++) {
                    char c = seek();
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                        (c >= 'A' && c <= 'F')) {
                        num_buf[i] = c;
                        next();
                    }
                    else {
                        break;
                    }
                }
                if (!sscanf(num_buf, "%2x", &value)) {
                    raise_error(get_state(), "Invalid hex escape sequence.");
                    set_state(state);
                    free(buf);
                    return NULL;
                }
                buf[i++] = (uint8_t) value & 0xFF;
                break;
            }
            default: {
                unsigned int value;
                char num_buf[4] = {0};
                for (int i = 0; i < 3; i++) {
                    char c = seek();
                    if (c >= '0' && c <= '7') {
                        num_buf[i] = c;
                        next();
                    }
                    else {
                        break;
                    }
                }
                if (!sscanf(num_buf, "%3o", &value)) {
                    raise_error(get_state(), "Invalid escape sequence.");
                    set_state(state);
                    free(buf);
                    return NULL;
                }
                buf[i++] = (uint8_t) value & 0xFF;
                break;
            }
            }
        }
        else {
            buf[i++] = next();
        }
    }

    Tag_string_t *tag = new_string(i);
    memcpy(tag->load, buf, i);
    free(buf);
    return (Tag_t *) tag;
}

Tag_t *parse_TAG_List()
{
    int state = get_state();
    uint8_t type = 0;
    int types_tried[TOTAL_TYPES] = {0};
    int i = 0;
    List_node_t *list = new_nodes_list();

    if (seek() == '[') {
        next();
    }
    else {
        raise_error(state, "Expected bracket.");
        set_state(state);
        return NULL;
    }

    int first_state = get_state();

    skip_whitespace();
    if (seek() == ']') {
        next();
        Tag_list_t *tag = finalise_nodes_list(0, NULL);
        return (Tag_t *) tag;
    }

    while (1) {
        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        int this_state = get_state();
        if (!type) {
            // First ever element (checking type)

            Tag_t *this = parse_any_data();
            if (this) {
                // Succeeded parsing element

                type = this->type;
                list = add_list_node(list, this);
                i++;
            }
            else {
                // Failed parsing element
                append_error(get_state(),
                             "Expected a valid element or closing brackets.");
                set_state(state);
                return NULL;
            }
        }
        else {
            // Subsequent elements (that already have a known type)

            Tag_t *this = function_table[type]();
            if (this) {
                // Possibly succeeded in parsing element

                list = add_list_node(list, this);
                i++;

                skip_whitespace();
                if (seek() == ']') {
                    next();
                    break;
                }
                else if (seek() == ',') {
                    next();
                    continue;
                }
            }

            // Failed parsing element (check new possible type)

            set_state(this_state);
            types_tried[type] = get_state();
            this = parse_any_data();
            if (this) {
                // Succeeded parsing element with new type

                free_tag_list((Tag_t *) finalise_nodes_list(type, list));
                list = new_nodes_list();
                type = this->type;
                if (types_tried[type]) {
                    // Type already been tested
                    raise_error(types_tried[type], "List is not homogeneous.");
                    free_tag_list((Tag_t *) finalise_nodes_list(0, list));
                    return NULL;
                }
                else {
                    // Reparse list with new type
                    types_tried[type] = get_state();
                    set_state(first_state);
                    i = 0;
                    continue;
                }
            }
            else {
                // Malformed element
                append_error(get_state(), "Expected a valid element.");
                set_state(state);
                free_tag_list((Tag_t *) finalise_nodes_list(type, list));
                return NULL;
            }
        }

        int comma_state = get_state();
        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (seek() == ',') {
            next();
        }
        else {
            raise_error(comma_state, "Expected a comma or closing brackets.");
            set_state(state);
            free_tag_list((Tag_t *) finalise_nodes_list(type, list));
            return NULL;
        }
    }

    Tag_list_t *tag = finalise_nodes_list(type, list);
    return (Tag_t *) tag;
}

Tag_t *parse_TAG_Compound()
{
    int state = get_state();
    Compound_node_t *list = new_compound_list();

    if (seek() == '{') {
        next();
    }
    else {
        raise_error(state, "Invalid compound.");
        set_state(state);
        return NULL;
    }

    skip_whitespace();
    if (seek() == '}') {
        next();
        Tag_compound_t *tag = new_compound(NULL);
        return (Tag_t *) tag;
    }
    while (1) {
        skip_whitespace();
        if (seek() == '}') {
            next();
            break;
        }
        Named_tag_t *this = parse_named_tag();
        if (this) {
            list = add_compound_node(list, this);
        }
        else {
            append_error(get_state(),
                         "Expected a valid tag or closing braces.");
            set_state(state);
            free_tag_compound((Tag_t *) new_compound(list));
            return NULL;
        }

        int comma_state = get_state();
        skip_whitespace();
        if (seek() == '}') {
            next();
            break;
        }
        else if (seek() == ',') {
            next();
        }
        else {
            raise_error(comma_state, "Expected a comma or closing braces.");
            set_state(state);
            free_tag_compound((Tag_t *) new_compound(list));
            return NULL;
        }
    }

    Tag_compound_t *tag = new_compound(list);
    return (Tag_t *) tag;
}

Tag_t *parse_TAG_Int_Array()
{
    int state = get_state();
    int32_t length = 0;
    int32_t *buf = NULL;

    if (!cmp_next("[I;")) {
        raise_error(state, "Invalid int array.");
        return NULL;
    }

    int i = 0;
    skip_whitespace();
    if (seek() == ']') {
        next();
        Tag_int_array_t *tag = new_int_array(i);
        free(buf);
        return (Tag_t *) tag;
    }
    while (1) {
        if (i >= length) {
            length += CHUNK;
            buf = realloc(buf, length * sizeof(uint32_t));
        }

        int local_state = get_state();
        uint8_t read[8];

        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (seek() == '-')
            next();
        while (1) {
            char c = seek();
            if (c < '0' || c > '9') break;
            next();
        }

        if (local_state == get_state()) {
            raise_error(get_state(), "No digits found.");
            set_state(state);
            free(buf);
            return NULL;
        }
        if (!sscanf(out_buf + local_state, " %ld", (long *) read)) {
            raise_error(get_state(), "Not a valid int.");
            set_state(state);
            free(buf);
            return NULL;
        }
        buf[i] = *(long *) read;

        i++;
        int comma_state = get_state();
        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (seek() == ',') {
            next();
        }
        else {
            raise_error(comma_state, "Expected a comma or closing brackets.");
            set_state(state);
            free(buf);
            return NULL;
        }
    }

    Tag_int_array_t *tag = new_int_array(i);
    memcpy(tag->load, buf, i * sizeof(uint32_t));
    free(buf);
    return (Tag_t *) tag;
}

Tag_t *parse_TAG_Long_Array()
{
    int state = get_state();
    int32_t length = 0;
    int64_t *buf = NULL;

    if (!cmp_next("[L;")) {
        raise_error(state, "Invalid long array.");
        return NULL;
    }
    int i = 0;
    skip_whitespace();
    if (seek() == ']') {
        next();
        Tag_long_array_t *tag = new_long_array(i);
        free(buf);
        return (Tag_t *) tag;
    }
    while (1) {
        if (i >= length) {
            length += CHUNK;
            buf = realloc(buf, length * sizeof(uint64_t));
        }

        int local_state = get_state();
        uint8_t read[8];

        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (seek() == '-')
            next();
        while (1) {
            char c = seek();
            if (c < '0' || c > '9') break;
            next();
        }
        if (seek() == 'l' || seek() == 'L') next();

        if (local_state == get_state()) {
            raise_error(get_state(), "No digits found.");
            set_state(state);
            free(buf);
            return NULL;
        }
        if (!sscanf(out_buf + local_state, " %ld", (long *) read)) {
            raise_error(get_state(), "Not a valid long.");
            set_state(state);
            free(buf);
            return NULL;
        }
        buf[i] = *(long *) read;

        i++;
        int comma_state = get_state();
        skip_whitespace();
        if (seek() == ']') {
            next();
            break;
        }
        else if (seek() == ',') {
            next();
        }
        else {
            raise_error(comma_state, "Expected a comma or closing brackets.");
            set_state(state);
            free(buf);
            return NULL;
        }
    }

    Tag_long_array_t *tag = new_long_array(i);
    memcpy(tag->load, buf, i * sizeof(uint64_t));
    free(buf);
    return (Tag_t *) tag;
}

static void parser_init()
{
    buf_len = 0;
    buf_index = 0;
    out_buf = NULL;
    do {
        out_buf = (char *) realloc(out_buf, buf_len + CHUNK);
        buf_len += fread(out_buf + buf_len, 1, CHUNK, stdin);
    } while (!feof(stdin));
}

static void parser_end()
{
    free(out_buf);
}

static uint8_t next()
{
    if (buf_index >= buf_len) {
        raise_error(get_state(), "ERROR! Unexpected EOF.");
        return -1;
    }
    return out_buf[buf_index++];
}

static uint8_t seek()
{
    if (buf_index >= buf_len) {
        raise_error(get_state(), "ERROR! Unexpected EOF.");
        raise_error(buf_index, "ERROR! Unexpected EOF.");
        return -1;
    }
    return out_buf[buf_index];
}

static uint8_t cmp_next(const char *str)
{
    int state = get_state();
    for (int i = 0; str[i]; i++) {
        if (next() != str[i]) {
            set_state(state);
            return 0;
        }
    }
    return 1;
}

static void skip_whitespace()
{
    char c = seek();
    while (1) {
        if (c != ' ' && c != '\t' && c != '\n') break;
        next();
        c = seek();
    }
}

static inline int get_state()
{
    return buf_index;
}

static inline void set_state(int state)
{
    buf_index = state;
}

void raise_error(int location, const char *message)
{
    free_error(global_error);

    error_t *new = (error_t *) malloc(sizeof(error_t));
    new->previous = NULL;
    new->message = message;
    new->location = location;

    global_error = new;
}

void append_error(int location, const char *message)
{
    error_t *new = (error_t *) malloc(sizeof(error_t));
    new->previous = global_error;
    new->message = message;
    new->location = location;

    global_error = new;
}

inline error_t *get_error()
{
    return global_error;
}

inline void clear_error()
{
    free_error(global_error);
    global_error = NULL;
}

static void free_error(error_t *error)
{
    if (!error) return;
    free_error(error->previous);
    free(error);
}

void print_error(error_t *error)
{
    if (!error) return;
    print_error(error->previous);
    if (error->previous) fprintf(stderr, _ERR "Which caused: ");

    int location = error->location;
    int line = 1, column = 1;
    for (int i = 0; i < location; i++) {
        column++;
        if (out_buf[i] == '\n') {
            line++;
            column = 1;
        }
    }
    fprintf(stderr,
            _ERR "Error! " _CLEAR "%s\n" _ERR
                 "- at line %d, column %d.\n" _CLEAR,
            error->message, line, column);
    for (int i = -10; i < 10; i++) {
        int idx = location + i;
        char c;
        if (idx < 0 || idx >= buf_len) {
            c = '.';
        }
        else if (out_buf[location + i] == '\t'
            || out_buf[location + i] == '\n')
        {
            c = ' ';
        }
        else {
            c = out_buf[location + i];
        }
        putc(c, stderr);
    }
    fprintf(stderr, _ERR "\n          ^\n\n");
}
