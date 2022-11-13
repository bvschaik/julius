#include "xml_parser.h"

#include "core/log.h"

#include "expat.h"

#include <string.h>

#define XML_HASH_SEED 0x12345678

static struct {
    xml_parser_element *elements;
    const xml_parser_element **parents;
    int depth;
    int total_elements;
    int error;
    XML_Parser parser;
    const xml_parser_element *current_element;
} data;

static int dummy_element_on_enter(const char **attributes, int total_attributes)
{
    return 1;
}

static void dummy_element_on_exit(void)
{}

static const xml_parser_element *get_element_from_name(const char *name)
{
    if (!name) {
        return 0;
    }
    for (int i = 0; i < data.total_elements; i++) {
        if (strcmp(name, data.elements[i].name) == 0) {
            return &data.elements[i];
        }
    }
    return 0;
}

static int compare_multiple(const char *string, const char *match)
{
    const char *next;
    do {
        next = strchr(string, '|');
        int length = next ? (next - string) : strlen(string);
        if (strncmp(string, match, length) == 0) {
            return 1;
        }
        string = next + 1;
    } while (next);
    return 0;
}

static int is_proper_child(const xml_parser_element *element)
{
    if (data.current_element == 0) {
        return element->parent_names == 0;
    }
    if (element->parent_names == 0) {
        return 0;
    }
    return compare_multiple(element->parent_names, data.current_element->name);
}

static int count_attributes(const char **attributes)
{
    int total = 0;
    while (attributes[total]) {
        ++total;
    }
    return total;
}

static void XMLCALL start_element(void *unused, const char *name, const char **attributes)
{
    if (data.error) {
        return;
    }
    const xml_parser_element *element = get_element_from_name(name);
    if (!element || !is_proper_child(element)) {
        data.error = 1;
        log_error("Invalid XML parameter", name, 0);
        XML_StopParser(data.parser, XML_FALSE);
        return;
    }
    data.current_element = element;
    data.parents[data.depth] = element;
    data.depth++;
    if (!element->on_enter(attributes, count_attributes(attributes))) {
        data.error = 1;
    }
    if (data.error) {
        XML_StopParser(data.parser, XML_FALSE);
    }
}

static void XMLCALL end_element(void *unused, const char *name)
{
    if (data.error) {
        return;
    }
    if (!data.current_element || strcmp(data.current_element->name, name) != 0) {
        data.error = 1;
        log_error("Invalid XML parameter", name, 0);
        XML_StopParser(data.parser, XML_FALSE);
        return;
    }
    data.current_element->on_exit();
    if (data.depth > 1) {
        data.depth--;
        data.current_element = data.parents[data.depth - 1];
    } else {
        data.current_element = 0;
    }
}

int xml_parser_init(const xml_parser_element *elements, int total_elements)
{
    xml_parser_free();

    size_t elements_size = sizeof(xml_parser_element) * total_elements;
    data.elements = malloc(elements_size);
    data.parents = malloc(sizeof(xml_parser_element *) * total_elements);
    if (!data.elements) {
        data.error = 1;
        return 0;
    }
    data.total_elements = total_elements;

    memcpy(data.elements, elements, elements_size);

    data.parser = XML_ParserCreate(NULL);
    XML_SetHashSalt(data.parser, XML_HASH_SEED);
    XML_SetElementHandler(data.parser, start_element, end_element);
    
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

int xml_parser_parse(const char *buffer, int buffer_size, int is_final)
{
    if (data.error) {
        return 0;
    }
    if (XML_Parse(data.parser, buffer, (int) buffer_size, is_final) == XML_STATUS_ERROR) {
        data.error = 1;
        return 0;
    }
    return 1;
}

static const char *get_attribute_value(const char **attributes, const char *key)
{
    if (!key) {
        return 0;
    }
    int i = 0;
    while (attributes[i]) {
        if (!attributes[i + 1]) {
            return 0;
        }
        if (strcmp(attributes[i], key) == 0) {
            return attributes[i + 1];
        }
        i += 2;
    }
    return 0;
}

int xml_parser_has_attribute(const char **attributes, const char *key)
{
    return get_attribute_value(attributes, key) != 0;
}

int xml_parser_get_attribute_int(const char **attributes, const char *key)
{
    const char *value = get_attribute_value(attributes, key);
    if (!value) {
        return 0;
    }
    return atoi(value);
}

const char *xml_parser_get_attribute_string(const char **attributes, const char *key)
{
    return get_attribute_value(attributes, key);
}

char *xml_parser_copy_attribute_string(const char **attributes, const char *key)
{
    const char *value = get_attribute_value(attributes, key);
    if (!value) {
        return 0;
    }
    size_t value_size = strlen(value);
    char *result = malloc(sizeof(char) * (value_size + 1));
    if (!result) {
        return 0;
    }
    strncpy(result, value, value_size + 1);
    return result;
}

int xml_parser_get_attribute_bool(const char **attributes, const char *key)
{
    const char *value = get_attribute_value(attributes, key);
    if (!value) {
        return 0;
    }
    return strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, key) == 0 ||
        strcmp(value, "yes") == 0 || strcmp(value, "y") == 0;
}

int xml_parser_get_attribute_enum(const char **attributes, const char *key,
    const char **values, int total_values, int start_offset)
{
    const char *value = get_attribute_value(attributes, key);
    if (!value) {
        return start_offset - 1;
    }
    for (int i = 0; i < total_values; i++) {
        if (values[i] && compare_multiple(values[i], value)) {
            return i + start_offset;
        }
    }
    return start_offset - 1;
}

void xml_parser_reset(void)
{
    data.error = 0;
    XML_ParserReset(data.parser, NULL);
    XML_SetHashSalt(data.parser, XML_HASH_SEED);
    XML_SetElementHandler(data.parser, start_element, end_element);

    data.depth = 0;
    data.current_element = 0;
    memset(data.parents, 0, sizeof(xml_parser_element *) * data.total_elements);
}

void xml_parser_free(void)
{
    XML_ParserFree(data.parser);
    data.parser = 0;
    data.error = 0;

    free(data.elements);
    free(data.parents);
    data.elements = 0;
    data.parents = 0;
    data.total_elements = 0;
    data.depth = 0;
    data.current_element = 0;
}
