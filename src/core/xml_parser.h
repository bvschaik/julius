#ifndef CORE_XML_PARSER_H
#define CORE_XML_PARSER_H

#define XML_PARSER_MAX_ATTRIBUTES 13
#define XML_PARSER_TAG_MAX_LENGTH 12

typedef struct {
    const char *name;
    int (*on_enter)(const char **attributes, int total_attributes);
    void (*on_exit)(void);
    const char *parent_names;
} xml_parser_element;

int xml_parser_init(const xml_parser_element *elements, int total_elements);

int xml_parser_parse(const char *buffer, int buffer_size, int is_final);

int xml_parser_has_attribute(const char **attributes, const char *key);
int xml_parser_get_attribute_int(const char **attributes, const char *key);
const char *xml_parser_get_attribute_string(const char **attributes, const char *key);
char *xml_parser_copy_attribute_string(const char **attributes, const char *key);
int xml_parser_get_attribute_bool(const char **attributes, const char *key);
int xml_parser_get_attribute_enum(const char **attributes, const char *key,
    const char **values, int total_values, int start_offset);

void xml_parser_reset(void);

void xml_parser_free(void);

#endif // CORE_XML_PARSER_H
