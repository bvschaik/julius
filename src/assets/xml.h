#ifndef ASSETS_XML_H
#define ASSETS_XML_H

void xml_init(void);
int xml_process_assetlist_file(const char *xml_file_name);
void xml_finish(void);
void xml_get_full_image_path(char *full_path, const char *image_file_name);

#endif // ASSETS_XML_H
