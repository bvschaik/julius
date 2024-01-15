#include "xml_exporter.h"

#include "core/encoding.h"
#include "core/string.h"

#include <stdio.h>
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

static void finish_start_tag(int has_children, int add_newline_for_children)
{
    if (data.current_element->start_tag_done == 1) {
        return;
    }

    data.current_element->start_tag_done = 1;
    if (has_children) {
        buffer_write_raw(data.output_buf, ">", 1);
        if (add_newline_for_children) {
            xml_exporter_newline();
        }
    } else {
        buffer_write_raw(data.output_buf, "/>", 2);
        xml_exporter_newline();

        data.current_element->element_closed_off = 1;
        decrease_depth();
    }
}

static void create_end_tag(int keep_inline)
{
    data.current_element->element_closed_off = 1;
    
    if (!keep_inline) {
        xml_exporter_whitespaces(data.current_element_depth * WHITESPACES_PER_TAB);
    }
    buffer_write_raw(data.output_buf, "</", 2);
    buffer_write_raw(data.output_buf, data.current_element->name, strlen(data.current_element->name));
    buffer_write_raw(data.output_buf, ">", 1);
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

void xml_exporter_new_element(const char *name, int is_child)
{
    if (is_child) {
        finish_start_tag(1, 1);
        increase_depth();
    } else {
        xml_exporter_close_element(0);
        same_depth_new_element();
    }
    data.current_element->name = name;
    
    xml_exporter_whitespaces(data.current_element_depth * WHITESPACES_PER_TAB);
    buffer_write_raw(data.output_buf, "<", 1);
    buffer_write_raw(data.output_buf, data.current_element->name, strlen(data.current_element->name));
}

void xml_exporter_add_attribute_text(const char *name, const uint8_t *value)
{
    xml_exporter_whitespaces(1);
    buffer_write_raw(data.output_buf, name, strlen(name));
    buffer_write_raw(data.output_buf, "=\"", 2);
#ifndef BUILDING_ASSET_PACKER
    char value_out[300];
    encoding_to_utf8(value, value_out, 300, 0);
    buffer_write_raw(data.output_buf, value, strlen(value_out));
#else
    const char *value_ascii = string_from_ascii(value);
    buffer_write_raw(data.output_buf, value_ascii, strlen(value_ascii));
#endif
    buffer_write_raw(data.output_buf, "\"", 1);
}

void xml_exporter_add_attribute_int(const char *name, int value)
{
    uint8_t attr_value[200];
    int value_length = string_from_int(attr_value, value, 0);

    xml_exporter_whitespaces(1);
    buffer_write_raw(data.output_buf, name, strlen(name));
    buffer_write_raw(data.output_buf, "=\"", 2);
    buffer_write_raw(data.output_buf, attr_value, value_length);
    buffer_write_raw(data.output_buf, "\"", 1);
}

void xml_exporter_add_text(const uint8_t *value)
{
    buffer_write_raw(data.output_buf, value, string_length(value));
}

void xml_exporter_add_element_text(const uint8_t *value)
{
    if (data.current_element->start_tag_done == 0) {
        finish_start_tag(1, 0);
    }
    xml_exporter_add_text(value);
}

void xml_exporter_close_element(int keep_inline)
{
    if (data.current_element) {
        if (data.current_element->start_tag_done == 0) {
            finish_start_tag(0, 1);
        } else if (data.current_element->element_closed_off == 0) {
            create_end_tag(keep_inline);
        }
    }
}

void xml_exporter_whitespaces(int count)
{
    for (int i = 0; i < count; i++) {
        buffer_write_raw(data.output_buf, " ", 1);
    }
}

void xml_exporter_newline(void)
{
    buffer_write_raw(data.output_buf, "\n", 1);
}
