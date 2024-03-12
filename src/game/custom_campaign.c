#include "custom_campaign.h"

#include "core/encoding.h"
#include "core/file.h"
#include "core/log.h"
#include "core/xml_parser.h"

#include "zip/zip.h"

#define XML_TOTAL_ELEMENTS 5

static int xml_start_campaign(void);
static int xml_start_intro(void);
static void xml_intro_text(const char *text);
static int xml_start_missions(void);
static int xml_start_mission(void);
static int xml_start_option(void);

static void xml_end_mission(void);

static struct {
    int success;
    custom_campaign_info campaign;
    char file_name[FILE_NAME_MAX];
} data;

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "campaign", xml_start_campaign },
    { "intro", xml_start_intro, 0, "campaign", xml_intro_text },
    { "missions", xml_start_missions, 0, "campaign" },
    { "mission", xml_start_mission, xml_end_mission, "missions" },
    { "option", xml_start_option, 0, "mission" }
};


static int get_info(const char *xml, unsigned int size)
{
    data.success = 1;

    if (!xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS)) {
        return 0;
    }
    if (!xml_parser_parse(xml, size, 1)) {
        data.success = 0;
    }
    xml_parser_free();

    return data.success;
}

static int xml_start_campaign(void)
{
    return xml_parser_get_attribute_int("version") == 1;
}

static const uint8_t *copy_string_from_xml(const char *text)
{
    if (!text || !*text) {
        return 0;
    }
    size_t length = strlen(text) + 1;
    uint8_t *result = malloc(sizeof(uint8_t) * length);
    if (!result) {
        return 0;
    }
    encoding_from_utf8(text, result, (int) length);
    return result;
}

static int xml_start_intro(void)
{
    data.campaign.name = copy_string_from_xml(xml_parser_get_attribute_string("title"));
    return 1;
}

static void xml_intro_text(const char *text)
{
    data.campaign.description = copy_string_from_xml(text);
}

static int xml_start_missions(void)
{
    return 1;
}

static int xml_start_mission(void)
{
    return 1;
}

static int xml_start_option(void)
{
    return 1;
}

static void xml_end_mission(void)
{
}

static char *extract_xml(struct zip_t *zip, unsigned int *size)
{
    *size = 0;
    if (zip_entry_open(zip, "settings.xml") < 0) {
        log_error("Unable to open campaign file - invalid file format", 0, 0);
        return 0;
    }

    size_t xml_size = zip_entry_size(zip);
    char *xml_text = malloc(xml_size);

    if (!xml_text) {
        log_error("Not enough memory to obtain the xml entry from the file", 0, 0);
        zip_entry_close(zip);
        return 0;
    }

    zip_entry_noallocread(zip, xml_text, xml_size);
    zip_entry_close(zip);

    *size = (unsigned int) xml_size;

    return xml_text;
}

const custom_campaign_info *custom_campaign_load_info(const char *filename)
{
    if (!filename || !*filename) {
        return 0;
    }

    if (strcmp(filename, data.file_name) == 0) {
        return &data.campaign;
    }

    custom_campaign_clear();

    log_info("Opening campaign file:", filename, 0);

    FILE *zip_stream = file_open(filename, "rb");

    if (!zip_stream) {
        log_error("Unable to open campaign file - file does not exist", 0, 0);
        return 0;
    }

    struct zip_t *zip = zip_cstream_open(zip_stream, 0, 'r');
    if (!zip) {
        log_error("Unable to open campaign file - invalid file format", 0, 0);
        return 0;
    }

    unsigned int xml_size;
    char *xml_text = extract_xml(zip, &xml_size);

    if (!xml_text) {
        zip_close(zip);
        file_close(zip_stream);

        return 0;
    }

    if (!get_info(xml_text, xml_size)) {
        log_error("Unable to open campaign settings - invalid file format", 0, 0);

        free(xml_text);

        zip_entry_close(zip);
        zip_close(zip);
        file_close(zip_stream);

        return 0;
    }

    free(xml_text);

    zip_entry_close(zip);
    zip_close(zip);
    file_close(zip_stream);

    strncpy(data.file_name, filename, FILE_NAME_MAX);

    return &data.campaign;
}

void custom_campaign_clear(void)
{
    if (data.campaign.name) {
        free((uint8_t *) data.campaign.name);
    }
    if (data.campaign.description) {
        free((uint8_t *) data.campaign.description);
    }
    memset(&data.campaign, 0, sizeof(data.campaign));

    data.file_name[0] = 0;
}
