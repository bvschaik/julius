#include "xml_parser.h"

#include "core/log.h"

#include "expat.h"

#include <string.h>

#define XML_HASH_SEED 0x12345678

static struct {
    xml_parser_element *elements;
    int total_elements;
    int error;
    XML_Parser parser;
    const xml_parser_element *current_element;
} data;

static void dummy_element_on_exit(void)
{
}

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

static int is_proper_child(const xml_parser_element *element)
{
    if (data.current_element == 0) {
        return element->parent_name == 0;
    }
    if (element->parent_name == 0) {
        return 0;
    }
    return strcmp(data.current_element->name, element->parent_name) == 0;
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
        return;
    }
    data.current_element = element;
    if (!element->on_enter(element, attributes, count_attributes(attributes))) {
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
    data.current_element = get_element_from_name(data.current_element->parent_name);
}

int xml_parser_init(const xml_parser_element *elements, int total_elements)
{
    xml_parser_free();

    size_t elements_size = sizeof(xml_parser_element) * total_elements;
    data.elements = malloc(elements_size);
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
        if (!element->name || !element->on_enter) {
            xml_parser_free();
            data.error = 1;
            return 0;
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

void xml_parser_reset(void)
{
    data.error = 0;
    XML_ParserReset(data.parser, NULL);
    XML_SetHashSalt(data.parser, XML_HASH_SEED);
    XML_SetElementHandler(data.parser, start_element, end_element);
}

void xml_parser_free(void)
{
    XML_ParserFree(data.parser);
    data.parser = 0;
    data.error = 0;

    free(data.elements);
    data.elements = 0;
    data.total_elements = 0;
}