#ifndef ASSETS_XML_H
#define ASSETS_XML_H

#define XML_STRING_MAX_LENGTH 32

void xml_process_assetlist_file(const char *xml_file_name);
void xml_get_full_image_path(char *full_path, const char *image_file_name);

#endif // ASSETS_XML_H
