#include "file.h"

#include "core/file.h"

#include "zip/zip.h"

#define CAMPAIGNS_PREFIX_SIZE sizeof(CAMPAIGNS_DIRECTORY)

static struct {
    int is_folder;
    char file_name[FILE_NAME_MAX];
    int file_name_offset;
    struct {
        FILE *stream;
        struct zip_t *parser;
    } zip;
} data;

int campaign_file_exists(const char *filename)
{
    if (data.is_folder) {
        snprintf(&data.file_name[data.file_name_offset], FILE_NAME_MAX - data.file_name_offset, "/%s", filename);
        return dir_get_file_at_location(data.file_name, PATH_LOCATION_CAMPAIGN) != 0;
    }
    int close_at_end = data.zip.parser == 0;
    if (!campaign_file_open_zip()) {
        return 0;
    }
    int has_file = zip_entry_open(data.zip.parser, filename) == 0;
    zip_entry_close(data.zip.parser);
    if (close_at_end) {
        campaign_file_close_zip();
    }
    return has_file;
}

static void *load_file_from_folder(const char *file, size_t *length)
{
    *length = 0;
    snprintf(&data.file_name[data.file_name_offset], FILE_NAME_MAX - data.file_name_offset, "/%s", file);
    const char *filename = dir_get_file_at_location(data.file_name, PATH_LOCATION_CAMPAIGN);
    if (!filename) {
        return 0;
    }
    FILE *campaign_file = file_open(filename, "rb");
    if (!campaign_file) {
        return 0;
    }
    fseek(campaign_file, 0, SEEK_END);
    *length = ftell(campaign_file);
    fseek(campaign_file, 0, SEEK_SET);
    uint8_t *buffer = malloc(sizeof(char) * *length);
    if (!buffer) {
        file_close(campaign_file);
        return 0;
    }
    size_t result = fread(buffer, 1, *length, campaign_file);
    file_close(campaign_file);
    if (result != *length) {
        free(buffer);
        return 0;
    }
    return buffer;
}

static void *load_file_from_zip(const char *file, size_t *length)
{
    *length = 0;
    int close_at_end = data.zip.parser == 0;
    if (!campaign_file_open_zip()) {
        return 0;
    }

    if (zip_entry_open(data.zip.parser, file) < 0) {
        if (close_at_end) {
            campaign_file_close_zip();
        }
        return 0;
    }

    *length = zip_entry_size(data.zip.parser);
    uint8_t *buffer = malloc(*length);
    if (!buffer) {
        *length = 0;
        zip_entry_close(data.zip.parser);
        if (close_at_end) {
            campaign_file_close_zip();
        }
        return 0;
    }

    size_t result = zip_entry_noallocread(data.zip.parser, buffer, *length);
    zip_entry_close(data.zip.parser);
    if (close_at_end) {
        campaign_file_close_zip();
    }

    if (result != *length) {
        *length = 0;
        free(buffer);
        return 0;
    }
    return buffer;
}

void *campaign_file_load(const char *file, size_t *length)
{
    return data.is_folder ? load_file_from_folder(file, length) : load_file_from_zip(file, length);
}

void campaign_file_set_path(const char *path)
{
    campaign_file_close_zip();
    if (path && path[0]) {
        data.is_folder = !file_has_extension(path, "campaign");
        data.file_name_offset = snprintf(data.file_name, FILE_NAME_MAX, "%s", path);
    } else {
        data.file_name[0] = 0;
        data.file_name_offset = 0;
        data.is_folder = 0;
    }
}

const char *campaign_file_remove_prefix(const char *path)
{
    if (!data.file_name[0]) {
        return 0;
    }
    if (strncmp(path, CAMPAIGNS_DIRECTORY "/", CAMPAIGNS_PREFIX_SIZE) != 0) {
        return 0;
    }
    path += CAMPAIGNS_PREFIX_SIZE;

    return path;
}

int campaign_file_is_zip(void)
{
    return !data.is_folder;
}

int campaign_file_open_zip(void)
{
    if (data.is_folder) {
        return 1;
    }
    if (!*data.file_name) {
        return 0;
    }
    if (!data.zip.stream) {
        const char *filename = dir_get_file_at_location(data.file_name, PATH_LOCATION_CAMPAIGN);
        if (!filename) {
            return 0;
        }
        data.zip.stream = file_open(filename, "rb");
        if (!data.zip.stream) {
            return 0;
        }
    }
    if (!data.zip.parser) {
        data.zip.parser = zip_cstream_open(data.zip.stream, 0, 'r');
        if (!data.zip.parser) {
            campaign_file_close_zip();
            return 0;
        }
    }
    return 1;
}

void campaign_file_close_zip(void)
{
    if (data.zip.parser) {
        zip_close(data.zip.parser);
        data.zip.parser = 0;
    }
    if (data.zip.stream) {
        file_close(data.zip.stream);
        data.zip.stream = 0;
    }
}
