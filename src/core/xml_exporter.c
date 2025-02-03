#include "xml_exporter.h"

#include "core/encoding.h"
#include "core/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WHITESPACES_PER_TAB 4
#define MAX_XML_TREE_DEPTH 20

static struct {
    int success;

    buffer *output_buf;
    xml_exporter_element *current_element;
    int current_element_depth;
    xml_exporter_element current_path[MAX_XML_TREE_DEPTH];
} data;

static int increase_depth(void)
{
    data.current_element_depth++;
    data.current_element = &data.current_path[data.current_element_depth];
    data.current_element->start_tag_done = 0;
    data.current_element->element_closed_off = 0;
    return data.current_element_depth;
}

static int decrease_depth(void)
{
    if (data.current_element_depth <= 0) {
        return 0;
    }
    
    data.current_element_depth--;
    data.current_element = &data.current_path[data.current_element_depth];
    return data.current_element_depth;
}

static void export_declaration_doctype(const char *type)
{
    buffer_write_raw(data.output_buf, "<?xml version=\"1.0\"?>", 21);
    xml_exporter_newline();
    buffer_write_raw(data.output_buf, "<!DOCTYPE ", 10);
    buffer_write_raw(data.output_buf, type, strlen(type));
    buffer_write_raw(data.output_buf, ">", 1);
    xml_exporter_newline();
}

static void finish_start_tag(void)
{
    if (data.current_element->start_tag_done) {
        return;
    }

    data.current_element->start_tag_done = 1;
    buffer_write_raw(data.output_buf, ">", 1);
    xml_exporter_newline();
}


static void add_whitespaces(int count)
{
    for (int i = 0; i < count; i++) {
        buffer_write_raw(data.output_buf, " ", 1);
    }
}

static void create_end_tag(void)
{
    if (data.current_element->element_closed_off) {
        return;
    }

    if (!data.current_element->start_tag_done) {
        buffer_write_raw(data.output_buf, "/>", 2);
        data.current_element->start_tag_done = 1;
    } else {
        add_whitespaces(data.current_element_depth * WHITESPACES_PER_TAB);

        buffer_write_raw(data.output_buf, "</", 2);
        buffer_write_raw(data.output_buf, data.current_element->name, strlen(data.current_element->name));
        buffer_write_raw(data.output_buf, ">", 1);
    }

    data.current_element->element_closed_off = 1;
    
    xml_exporter_newline();

    decrease_depth();
}

static int same_depth_new_element(void)
{
    data.current_element = &data.current_path[data.current_element_depth];
    data.current_element->start_tag_done = 0;
    data.current_element->element_closed_off = 0;
    return data.current_element_depth;
}

void xml_exporter_init(buffer *buf, const char *type)
{
    data.output_buf = buf;
    data.current_element = 0;
    data.current_element_depth = 0;
    data.success = 0;
    export_declaration_doctype(type);
}

void xml_exporter_new_element(const char *name)
{
    if (data.current_element) {
        if (!data.current_element->element_closed_off) {
            finish_start_tag();
            increase_depth();
        } else {
            xml_exporter_close_element();
            same_depth_new_element();
        }
    } else {
        same_depth_new_element(); 
    }
    data.current_element->name = name;
    
    add_whitespaces(data.current_element_depth * WHITESPACES_PER_TAB);
    buffer_write_raw(data.output_buf, "<", 1);
    buffer_write_raw(data.output_buf, data.current_element->name, strlen(data.current_element->name));
}

static size_t get_attribute_length(const char *attribute)
{
    const char *pos = strchr(attribute, '|');
    if (pos) {
        return pos - attribute;
    } else {
        return strlen(attribute);
    }
}

void xml_exporter_add_attribute_text(const char *name, const char *value)
{
    add_whitespaces(1);
    buffer_write_raw(data.output_buf, name, strlen(name));
    buffer_write_raw(data.output_buf, "=\"", 2);
    buffer_write_raw(data.output_buf, value, get_attribute_length(value));
    buffer_write_raw(data.output_buf, "\"", 1);
}

static const char *value_to_utf8(const uint8_t *value)
{
#ifndef BUILDING_ASSET_PACKER
    static int length;
    static char *value_utf8;
    int value_size = string_length(value) + 1;
    if (value_size > length) {
        char *new_temp_value = realloc(value_utf8, value_size);
        if (new_temp_value) {
            length = value_size;
            value_utf8 = new_temp_value;
        }
    }
    if (value_utf8) {
        encoding_to_utf8(value, value_utf8, length, 0);
        return value_utf8;
    } else {
        return (char *) value;
    }
#else
    return (char *) value;
#endif
}

void xml_exporter_add_attribute_encoded_text(const char *name, const uint8_t *value)
{
    xml_exporter_add_attribute_text(name, value_to_utf8(value));
}

void xml_exporter_add_attribute_int(const char *name, int value)
{
    uint8_t attr_value[200];
    int value_length = string_from_int(attr_value, value, 0);

    add_whitespaces(1);
    buffer_write_raw(data.output_buf, name, strlen(name));
    buffer_write_raw(data.output_buf, "=\"", 2);
    buffer_write_raw(data.output_buf, attr_value, value_length);
    buffer_write_raw(data.output_buf, "\"", 1);
}

void xml_exporter_add_text(const char *value)
{
    buffer_write_raw(data.output_buf, value, strlen(value));
}

void xml_exporter_add_element_text(const char *value)
{
    if (data.current_element->start_tag_done == 0) {
        finish_start_tag();
    }
    int single_line = !strchr(value, '\n');
    if (single_line) {
        add_whitespaces((data.current_element_depth + 1) * WHITESPACES_PER_TAB);
    }
    xml_exporter_add_text(value);
    if (single_line) {
        xml_exporter_newline();
    }
}

void xml_exporter_add_element_encoded_text(const uint8_t *value)
{
    xml_exporter_add_element_text(value_to_utf8(value));
}

void xml_exporter_close_element(void)
{
    if (data.current_element) {
        create_end_tag();
    }
}

void xml_exporter_newline(void)
{
    buffer_write_raw(data.output_buf, "\n", 1);
}
