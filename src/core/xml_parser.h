#ifndef CORE_XML_PARSER_H
#define CORE_XML_PARSER_H

#define XML_PARSER_MAX_ATTRIBUTES 13
#define XML_PARSER_TAG_MAX_LENGTH 12

typedef struct xml_parser_element {
    const char *name;
    const char attributes[XML_PARSER_MAX_ATTRIBUTES][XML_PARSER_TAG_MAX_LENGTH];
    int (*on_enter)(const struct xml_parser_element *element, const char **attributes, int total_attributes);
    void (*on_exit)(void);
    const char *parent_name;
} xml_parser_element;

int xml_parser_init(const xml_parser_element *elements, int total_elements);

int xml_parser_parse(const char *buffer, int buffer_size, int is_final);

void xml_parser_reset(void);

void xml_parser_free(void);

#endif // CORE_XML_PARSER_H
