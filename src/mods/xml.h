#ifndef MODS_XML_H
#define MODS_XML_H

#define XML_STRING_MAX_LENGTH 32

void xml_setup_base_folder_string(const char *base_folder);
void xml_process_mod_file(const char *xml_file_name);
void xml_get_current_full_path_for_image(char *full_path, const char *file_name);

#endif // MODS_XML_H
