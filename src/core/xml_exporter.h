#ifndef CORE_XML_EXPORTER_H
#define CORE_XML_EXPORTER_H

#include "core/buffer.h"

/*
 * @brief A structure that holds info about a xml export element.
 */
typedef struct {
    const char *name; /**< The name of the element. */
    int start_tag_done;
    int element_closed_off;
} xml_exporter_element;

/**
 * @brief Starts the generation of a new XML doc to the target buffer.
 * 
 * @param buf The output buffer that will be populated with the generated XML.
 * @param type The type of XML file this is.
 */
void xml_exporter_init(buffer *buf, const char *type);

/**
 * @brief Starts a new xml element, creating a start-tag.
 * 
 * @param name The name of the new element to start.
 * @param is_child Is this new element a child of the current path?
 */
void xml_exporter_new_element(const char *name, int is_child);

/**
 * @brief Exports a text attribute for the current tag.
 * 
 * @param name The name of the attribute.
 * @param value The value of the attribute.
 */
void xml_exporter_add_attribute_text(const char *name, const uint8_t *value);

/**
 * @brief Exports an integer attribute for the current tag.
 * 
 * @param name The name of the attribute.
 * @param value The value of the attribute.
 */
void xml_exporter_add_attribute_int(const char *name, int value);

/**
 * @brief Exports a text inside the current element. Will close the start tag if it is still open.
 * 
 * @param value The value of the attribute.
 */
void xml_exporter_add_element_text(const uint8_t *value);

/**
 * @brief Closes the current element,
 *        creating a end-tag (or ending the start-tag as an empty-element tag).
 * 
 * @param keep_inline Should the close tag be kept on the same line without whitespaces or a newline added?
 */
void xml_exporter_close_element(int keep_inline);

/**
 * @brief Adds the requested number of whitespaces to the buffer.
 * 
 * @param count How many whitespaces to add
 */
void xml_exporter_whitespaces(int count);

/**
 * @brief Adds a LF+CR (new line) to the buffer.
 */
void xml_exporter_newline(void);

#endif // CORE_XML_EXPORTER_H
