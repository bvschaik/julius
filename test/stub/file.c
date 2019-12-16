#include <string.h>

#ifdef _WIN32
#include <windows.h>

const char* filename_to_utf8(const wchar_t* str)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char* result = (char*)malloc(sizeof(char) * size_needed);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, size_needed, NULL, NULL);
    return result;
}
#endif

void platform_check_file_access_permissions(void)
{}

int platform_generate_full_file_path(char* full_path, const char* filepath)
{
    strcpy(full_path, filepath);
    return strlen(full_path);
}

const char* platform_get_base_path(void)
{
    return ".";
}
