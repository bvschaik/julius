#include "xml_parser.h"

#include "core/log.h"

#include "sxml/sxml.h"

#include <stdlib.h>
#include <string.h>

#define XML_TOKENS_SIZE_STEP 128
#define XML_ELEMENT_TEXT_BASE_LENGTH 64

typedef struct {
    char *text;
    int current_size;
    int capacity;
} element_text;

static struct {
    xml_parser_element *elements;
    const xml_parser_element **parents;
    int depth;
    int error_depth;
    int total_elements;
    int error;
    int stop_on_invalid_xml;
    struct {
        sxml_t context;
        sxmltok_t *tokens;
        unsigned int num_tokens;
        int line_number;
        unsigned int current_position;
    } parser;
    struct {
        char *data;
        int size;
        int cursor;
    } buffer;
    const xml_parser_element *current_element;
    struct {
        const sxmltok_t *first;
        int size;
    } attributes;
    element_text *texts;
} data;

static const char EMPTY_STRING = 0;

static int dummy_element_on_enter(void)
{
    return 1;
}

static void dummy_element_on_exit(void)
{}

int xml_parser_compare_multiple(const char *string, const char *match)
{
    const char *next;
    do {
        next = strchr(string, '|');
        size_t length = next ? (size_t) (next - string) : strlen(string);
        if (strncmp(string, match, length) == 0) {
            return 1;
        }
        string = next + 1;
    } while (next);
    return 0;
}

static void append_to_text(element_text *text_data, const char *text, int length)
{
    if (text_data->current_size + length > text_data->capacity) {
        int new_capacity = text_data->capacity ? text_data->capacity * 2 : XML_ELEMENT_TEXT_BASE_LENGTH;
        while (new_capacity < length + text_data->current_size) {
            new_capacity *= 2;
        }
        char *new_text = realloc(text_data->text, new_capacity);
        if (!new_text) {
            length = text_data->capacity - text_data->current_size;
        } else {
            text_data->text = new_text;
            text_data->capacity = new_capacity;
        }
    }
    if (length == 0) {
        return;
    }
    memcpy(text_data->text + text_data->current_size, text, length);
    text_data->current_size += length;
}

static void finish_text(void)
{
    if (!data.current_element || !data.current_element->on_text) {
        return;
    }
    element_text *current_text = &data.texts[data.depth];
    if (!current_text->text) {
        return;
    }
    // Remove trailing paragraph
    if (current_text->current_size > 0 && current_text->text[current_text->current_size - 1] == '\n') {
        current_text->current_size--;
    }
    append_to_text(current_text, "\0", 1);
    // Remove beginning empty text
    const char *text = current_text->text;
    while ((*text == '\r' || *text == '\n' || *text == ' ') && *text) {
        text++;
    }
    if (*text) {
        data.current_element->on_text(text);
    }
    free(current_text->text);
    memset(current_text, 0, sizeof(element_text));
}

static int is_proper_child(const xml_parser_element *element)
{
    if (data.current_element == 0) {
        return element->parent_names == 0;
    }
    if (element->parent_names == 0) {
        return 0;
    }
    return xml_parser_compare_multiple(element->parent_names, data.current_element->name);
}

static const xml_parser_element *get_element_from_name(const char *name)
{
    if (!name || !*name) {
        return 0;
    }
    for (int i = 0; i < data.total_elements; i++) {
        if (strcmp(name, data.elements[i].name) == 0 && is_proper_child(&data.elements[i])) {
            return &data.elements[i];
        }
    }
    return 0;
}

static int handle_attribute_value(const sxmltok_t *first, int limit)
{
    int position = 0;
    int i = 0;
    while (i < limit) {
        if (first[i].type != SXML_CHARACTER) {
            break;
        }
        position = first[i].endpos;
        i++;
    }
    if (position) {
        data.buffer.data[position] = 0;
    }
    return i;
}

static int handle_attributes(const sxmltok_t *first, unsigned int size)
{
    data.attributes.size = size;
    if (!size) {
        return 1;
    }
    data.attributes.first = first;
    for (unsigned int i = 0; i < size; i++) {
        data.buffer.data[first[i].endpos] = 0;
        i += handle_attribute_value(first + i + 1, size - i - 1);
    }
    return 1;
}

static void start_element(const sxmltok_t *token)
{
    if (data.error) {
        return;
    }
    finish_text();
    data.depth++;
    if (data.error_depth) {
        return;
    }
    const char *name = data.buffer.data + token->startpos;
    data.buffer.data[token->endpos] = 0;
    const xml_parser_element *element = get_element_from_name(name);
    if (!element) {
        data.error_depth = data.depth;
        log_error("Invalid XML element name", name, 0);
        if (data.stop_on_invalid_xml) {
            data.error = 1;
        }
        return;
    }
    data.current_element = element;

    if (!handle_attributes(token + 1, token->size)) {
        data.error_depth = data.depth;
        log_error("Malformed attributes for the element", element->name, 0);
    } else if (!element->on_enter()) {
        data.error_depth = data.depth;
    }
    if (!data.error_depth) {
        data.parents[data.depth - 1] = element;
    } else if (data.stop_on_invalid_xml) {
        data.error = 1;
    }
}

static void reduce_current_depth(void)
{
    if (data.depth > 1) {
        data.depth--;
        data.current_element = data.parents[data.depth - 1];
    } else {
        data.current_element = 0;
    }
}

static void end_element(const sxmltok_t *token)
{
    if (data.error) {
        return;
    }
    if (data.error_depth) {
        if (data.error_depth == data.depth) {
            data.error_depth = 0;
            data.attributes.first = 0;
            data.attributes.size = 0;
            reduce_current_depth();
        } else {
            data.depth--;
        }
        return;
    }
    const char *name = data.buffer.data + token->startpos;
    data.buffer.data[token->endpos] = 0;
    if (!data.current_element || strcmp(data.current_element->name, name) != 0) {
        data.error = 1;
        log_error("XML mismatch between element open and close", name, 0);
        return;
    }
    finish_text();
    data.attributes.first = 0;
    data.attributes.size = 0;
    data.current_element->on_exit();
    reduce_current_depth();
}

static void handle_element_text(const sxmltok_t *token)
{
    if (data.error_depth) {
        return;
    }
    if (data.current_element && data.current_element->on_text) {
        const char *text = data.buffer.data + token->startpos;
        int length = token->endpos - token->startpos;
        while (length) {
            char *end = memchr(text, '\n', length);
            int line_length = end ? end - text + 1: length;
            length -= line_length;
            // Remove whitespace at beginning
            while (*text == ' ' && line_length > 0) {
                text++;
                line_length--;
            }
            append_to_text(&data.texts[data.depth], text, line_length);
            text += line_length;
        }
    }
}

static int expand_xml_token_array(void)
{
    unsigned int expanded_size = data.parser.num_tokens + XML_TOKENS_SIZE_STEP;
    sxmltok_t *expanded_tokens = realloc(data.parser.tokens, sizeof(sxmltok_t) * expanded_size);
    if (!expanded_tokens) {
        return 0;
    }
    data.parser.tokens = expanded_tokens;
    data.parser.num_tokens = expanded_size;
    return 1;
}

int xml_parser_init(const xml_parser_element *elements, int total_elements, int stop_on_invalid_xml)
{
    xml_parser_free();

    size_t elements_size = sizeof(xml_parser_element) * total_elements;
    data.elements = malloc(elements_size);
    data.parents = malloc(sizeof(xml_parser_element *) * total_elements);
    data.texts = malloc(sizeof(element_text) * total_elements);
    data.stop_on_invalid_xml = stop_on_invalid_xml;
    
    if (!data.elements || !data.parents || !data.texts || !expand_xml_token_array()) {
        xml_parser_free();
        data.error = 1;
        return 0;
    }
    data.total_elements = total_elements;

    memcpy(data.elements, elements, elements_size);
    memset(data.parents, 0, sizeof(xml_parser_element *) * total_elements);
    memset(data.texts, 0, sizeof(element_text) * total_elements);

    sxml_init(&data.parser.context);

    for (int i = 0; i < data.total_elements; i++) {
        xml_parser_element *element = &data.elements[i];
        if (!element->name) {
            xml_parser_free();
            data.error = 1;
            return 0;
        }
        if (!element->on_enter) {
            element->on_enter = dummy_element_on_enter;
        }
        if (!element->on_exit) {
            element->on_exit = dummy_element_on_exit;
        }
    }
    return 1;
}

static void increase_line_count(unsigned int position)
{
    const char *cursor = data.buffer.data + data.parser.current_position;
    const char *end = data.buffer.data + position;

    for (int i = 0; cursor < end; i++, cursor++) {
        cursor = memchr(cursor, '\n', end - cursor);
        if (cursor == 0) {
            data.parser.line_number += i;
            break;
        }
    }
    data.parser.current_position = position;
}

static void process_xml_tokens(const sxmltok_t *tokens, int num_tokens)
{
    for (int i = 0; i < num_tokens; i++) {
        const sxmltok_t *token = &tokens[i];
        // Shouldn't happen
        if (i + token->size >= num_tokens) {
            data.error = 1;
            return;
        }
        increase_line_count(token->startpos);
        switch (token->type) {
            case SXML_STARTTAG:
                start_element(token);
                break;
            case SXML_ENDTAG:
                end_element(token);
                break;
            case SXML_CDATA:
            case SXML_CHARACTER:
                handle_element_text(token);
                break;
            default:
                break;
        }
        if (data.error) {
            return;
        }
        i += token->size;
    }
    increase_line_count(data.parser.context.bufferpos);
}

int fit_buffer(int size)
{
    if (data.buffer.size >= size) {
        return 1;
    }
    char *buffer = realloc(data.buffer.data, size * sizeof(char));

    if (!buffer) {
        return 0;
    }

    data.buffer.data = buffer;
    data.buffer.size = size;

    return 1;
}

int xml_parser_parse(const char *buffer, unsigned int buffer_size, int is_final)
{
    if (data.error || !data.parser.num_tokens) {
        return 0;
    }
    sxmlerr_t result;
    data.parser.context.bufferpos = 0;
    if (!fit_buffer(data.buffer.cursor + buffer_size)) {
        log_error("Out of memory", 0, 0);
        data.error = 1;
        return 0;
    }
    memcpy(data.buffer.data + data.buffer.cursor, buffer, buffer_size);
    data.parser.current_position = 0;
    do {
        data.parser.context.ntokens = 0;
        result = sxml_parse(&data.parser.context, data.buffer.data, buffer_size + data.buffer.cursor,
            data.parser.tokens, data.parser.num_tokens);
        if (data.parser.current_position == data.parser.context.bufferpos && !expand_xml_token_array()) {
            log_error("Unable to parse more data - token buffer full", 0, 0);
            data.error = 1;
            return 0;
        }
        if (result == SXML_ERROR_XMLINVALID) {
            increase_line_count(data.parser.context.bufferpos);
            log_error("XML parse error on line:", 0, xml_parser_get_current_line_number());
            data.error = 1;
            return 0;
        }
        process_xml_tokens(data.parser.tokens, data.parser.context.ntokens);

        if (data.error) {
            return 0;
        }
    } while (result == SXML_ERROR_TOKENSFULL);

    if (result == SXML_ERROR_BUFFERDRY) {
        if (is_final) {
            log_error("XML input file ended unexpectedly", 0, 0);
            data.error = 1;
            return 0;
        } else {
            data.buffer.cursor += buffer_size - data.parser.context.bufferpos;
            if (!fit_buffer(data.buffer.cursor + buffer_size)) {
                log_error("Out of memory", 0, 0);
                data.error = 1;
                return 0;
            }
            memmove(data.buffer.data, data.buffer.data + data.parser.context.bufferpos, data.buffer.cursor);
        }
    } else {
        data.buffer.cursor = 0;
    }

    return 1;
}

static const char *get_attribute_value(const char *key)
{
    if (!key || !data.attributes.first) {
        return 0;
    }
    for (int i = 0; i < data.attributes.size; i++) {
        const sxmltok_t *current = &data.attributes.first[i];
        if (current->type != SXML_CDATA) {
            continue;
        }
        if (strcmp(data.buffer.data + current->startpos, key) != 0) {
            continue;
        }
        if (i + 1 == data.attributes.size) {
            return &EMPTY_STRING;
        }
        current = &data.attributes.first[i + 1];
        if (current->type != SXML_CHARACTER) {
            return &EMPTY_STRING;
        }
        return data.buffer.data + current->startpos;
    }
    return 0;
}

int xml_parser_has_attribute(const char *key)
{
    return get_attribute_value(key) != 0;
}

int xml_parser_get_attribute_int(const char *key)
{
    const char *value = get_attribute_value(key);
    if (!value) {
        return 0;
    }
    return atoi(value);
}

const char *xml_parser_get_attribute_string(const char *key)
{
    return get_attribute_value(key);
}

char *xml_parser_copy_attribute_string(const char *key)
{
    const char *value = get_attribute_value(key);
    if (!value) {
        return 0;
    }
    size_t buf_size = sizeof(char) * (strlen(value) + 1);
    char *result = malloc(buf_size);
    if (!result) {
        return 0;
    }
    memcpy(result, value, buf_size);
    return result;
}

int xml_parser_get_attribute_bool(const char *key)
{
    const char *value = get_attribute_value(key);
    if (!value) {
        return 0;
    }
    return value == &EMPTY_STRING || strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, key) == 0 ||
        strcmp(value, "yes") == 0 || strcmp(value, "y") == 0;
}

int xml_parser_get_attribute_enum(const char *key,
    const char **values, int total_values, int start_offset)
{
    const char *value = get_attribute_value(key);
    if (!value) {
        return start_offset - 1;
    }
    for (int i = 0; i < total_values; i++) {
        if (values[i] && xml_parser_compare_multiple(values[i], value)) {
            return i + start_offset;
        }
    }
    return start_offset - 1;
}

int xml_parser_get_current_line_number(void)
{
    return data.parser.line_number + 1;
}

const char *xml_parser_get_current_element_name(void)
{
    return data.current_element->name;
}

const char *xml_parser_get_parent_element_name(void)
{
    if (data.depth < 2) {
        return 0;
    }
    return data.parents[data.depth - 2]->name;
}

void xml_parser_reset(void)
{
    data.error = 0;
    sxml_init(&data.parser.context);
    data.depth = 0;
    data.parser.line_number = 0;
    data.parser.current_position = 0;
    data.error_depth = 0;
    data.current_element = 0;
    memset(data.parents, 0, sizeof(xml_parser_element *) * data.total_elements);
    free(data.buffer.data);
    data.buffer.size = 0;
    data.buffer.cursor = 0;
    data.buffer.data = 0;
    data.attributes.first = 0;
    data.attributes.size = 0;
}

void xml_parser_free(void)
{
    data.error = 0;

    free(data.elements);
    free(data.parents);
    free(data.texts);
    data.elements = 0;
    data.parents = 0;
    data.texts = 0;
    data.total_elements = 0;
    data.depth = 0;
    data.parser.line_number = 0;
    data.parser.current_position = 0;
    data.error_depth = 0;
    data.current_element = 0;
    free(data.buffer.data);
    data.buffer.size = 0;
    data.buffer.cursor = 0;
    data.buffer.data = 0;
    free(data.parser.tokens);
    data.parser.num_tokens = 0;
    data.parser.tokens = 0;
    data.attributes.first = 0;
    data.attributes.size = 0;
}
