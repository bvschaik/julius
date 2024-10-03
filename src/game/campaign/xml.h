#ifndef CAMPAIGN_XML_H
#define CAMPAIGN_XML_H

#include "game/campaign.h"

#include <stdint.h>

int campaign_xml_get_info(const char *xml_text, size_t xml_size, campaign_info *info);

#endif // CAMPAIGN_XML_H
