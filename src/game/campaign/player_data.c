#include "player_data.h"

#include "core/array.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/xml_parser.h"
#include "core/xml_exporter.h"

#include <stdlib.h>

#define CAMPAIGNS_ARRAY_SIZE 5
#define XML_PARSER_ELEMENTS 2
#define XML_FILE_NAME "campaign_player_data.xml"

typedef struct {
    const char *file_name;
    int current_mission;
} campaign_player_data;

static struct {
    array(campaign_player_data) campaigns;
    int is_loaded;
} data;

static int xml_start_campaign(void);

static const xml_parser_element xml_elements[XML_PARSER_ELEMENTS] = {
    { "campaigns" },
    { "campaign", xml_start_campaign, 0, "campaigns" }
};

static int xml_start_campaign(void)
{
    if (!xml_parser_has_attribute("file_name") || !xml_parser_has_attribute("current_mission")) {
        return 1;
    }
    campaign_player_data *campaign = array_advance(data.campaigns);
    if (!campaign) {
        log_error("Problem creating a campaign element. Out of memory.", 0, 0);
        return 0;
    }
    campaign->file_name = xml_parser_copy_attribute_string("file_name");
    campaign->current_mission = xml_parser_get_attribute_int("current_mission");

    return 1;
}

static char *file_to_buffer(const char *filename, size_t *output_length)
{
    FILE *file = file_open(filename, "r");
    if (!file) {
        log_error("Error opening file", filename, 0);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = malloc(size);
    if (!buf) {
        log_error("Error creating memory for file", filename, 0);
        return 0;
    }

    *output_length = fread(buf, 1, size, file);
    if (*output_length > size) {
        log_error("Unable to read file into buffer", filename, 0);
        free(buf);
        file_close(file);
        *output_length = 0;
        return 0;
    }
    file_close(file);
    return buf;
}

static void load_campaign_player_data(void)
{
    if (data.is_loaded) {
        return;
    }
    if (!array_init(data.campaigns, CAMPAIGNS_ARRAY_SIZE, 0, 0)) {
        log_error("Problem creating the campaign player data structure. Out of memory.", 0, 0);
        return;
    }

    const char *xml_file = dir_get_file_at_location(XML_FILE_NAME, PATH_LOCATION_CONFIG);

    if (!xml_file) {
        data.is_loaded = 1;
        return;
    }

    size_t xml_file_length;

    char *xml_data = file_to_buffer(xml_file, &xml_file_length);
    if (!xml_data) {
        return;
    }

    if (!xml_parser_init(xml_elements, XML_PARSER_ELEMENTS, 0)) {
        free(xml_data);
        log_error("Problem creating the campaign player data xml parser.", 0, 0);
        return;
    }
    xml_parser_parse(xml_data, (unsigned int) xml_file_length, 1);
    xml_parser_free();
    free(xml_data);
    data.is_loaded = 1;
}

static campaign_player_data *get_campaign_data_for_file_name(const char *campaign_file_name)
{
    load_campaign_player_data();
    campaign_player_data *campaign;
    array_foreach(data.campaigns, campaign) {
        if (strcmp(campaign_file_name, campaign->file_name) == 0) {
            return campaign;
        }
    }
    return 0;
}

static void save_campaign_player_data(void)
{
    buffer buf;
    int buf_size = 1024 + 512 * data.campaigns.size;
    uint8_t *buf_data = malloc(buf_size);
    if (!buf_data) {
        log_error("Unable to save campaign player data. Out of memory", 0, 0);
        return;
    }
    buffer_init(&buf, buf_data, buf_size);
    xml_exporter_init(&buf, "campaign_player_data");
    xml_exporter_new_element("campaigns");
    const campaign_player_data *campaign;
    array_foreach(data.campaigns, campaign) {
        xml_exporter_new_element("campaign");
        xml_exporter_add_attribute_text("file_name", campaign->file_name);
        xml_exporter_add_attribute_int("current_mission", campaign->current_mission);
        xml_exporter_close_element();
    }
    xml_exporter_close_element();
    
    const char *xml_file = dir_append_location(XML_FILE_NAME, PATH_LOCATION_CONFIG);

    FILE *xml_dest = fopen(xml_file, "wb");

    if (!xml_dest) {
        log_error("Failed to create file", XML_FILE_NAME, 0);
        free(buf_data);
        return;
    }

    fwrite(buf.data, 1, buf.index, xml_dest);

    fclose(xml_dest);
    free(buf_data);
}

int campaign_player_data_get_current_mission(const char *campaign_file_name)
{
    campaign_player_data *campaign = get_campaign_data_for_file_name(campaign_file_name);
    return campaign ? campaign->current_mission : 0;
}

void campaign_player_data_update_current_mission(const char *campaign_file_name, int current_mission)
{
    campaign_player_data *campaign = get_campaign_data_for_file_name(campaign_file_name);
    if (campaign && campaign->current_mission >= current_mission) {
        return;
    }
    if (!campaign) {
        campaign = array_advance(data.campaigns);
        if (!campaign) {
            log_error("Problem creating a campaign element. Out of memory.", 0, 0);
            return;
        }
        char * new_file_name = malloc((strlen(campaign_file_name) + 1) * sizeof(char));
        if (!new_file_name) {
            log_error("Problem creating a campaign element. Out of memory.", 0, 0);
            return;
        }
        snprintf(new_file_name, strlen(campaign_file_name) + 1, "%s", campaign_file_name);
        campaign->file_name = new_file_name;
    }
    campaign->current_mission = current_mission;
    save_campaign_player_data();
}
