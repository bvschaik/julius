#include "data_transfer.h"

#include "building/roadblock.h"
#include "building/storage.h"
#include "city/warning.h"

#include <string.h>

static struct {
    building_data_type data_type;
    unsigned char resource[RESOURCE_MAX];
    building_storage storage;
    char i8;
    short i16;
    int i32;
} data;

int building_data_transfer_possible(building *b)
{
    building_data_type data_type = building_data_transfer_data_type_from_building_type(b->type);
    if (data.data_type == DATA_TYPE_NOT_SUPPORTED || data_type == DATA_TYPE_NOT_SUPPORTED) {
        city_warning_show(WARNING_DATA_PASTE_FAILURE, NEW_WARNING_SLOT);
        return 0;
    }
    if (data.data_type != data_type) {
        city_warning_show(WARNING_DATA_PASTE_FAILURE, NEW_WARNING_SLOT);
        return 0;
    }
    return 1;
}

int building_data_transfer_copy(building *b)
{
    building_data_type data_type = building_data_transfer_data_type_from_building_type(b->type);
    if (data_type == DATA_TYPE_NOT_SUPPORTED) {
        city_warning_show(WARNING_DATA_COPY_NOT_SUPPORTED, NEW_WARNING_SLOT);
    } else {
        memset(&data, 0, sizeof(data));
        data.data_type = data_type;
    }

    const building_storage *storage;

    switch (data_type) {
        case DATA_TYPE_ROADBLOCK:
            data.i16 = b->data.roadblock.exceptions;
            break;
        case DATA_TYPE_MARKET:
            memcpy(data.resource, b->accepted_goods, sizeof(unsigned char) * RESOURCE_MAX);
            break;
        case DATA_TYPE_GRANARY:
            storage = building_storage_get(b->storage_id);
            data.storage = *storage;
            break;
        case DATA_TYPE_WAREHOUSE:
            storage = building_storage_get(b->storage_id);
            data.storage = *storage;
            break;
        case DATA_TYPE_DOCK:
            memcpy(data.resource, b->accepted_goods, sizeof(unsigned char) * RESOURCE_MAX);
            data.i8 = b->data.dock.has_accepted_route_ids;
            data.i32 = b->data.dock.accepted_route_ids;
            break;
        default:
            return 0;

    }
    city_warning_show(WARNING_DATA_COPY_SUCCESS, NEW_WARNING_SLOT);
    return 1;
}

int building_data_transfer_paste(building *b)
{
    building_data_type data_type = building_data_transfer_data_type_from_building_type(b->type);

    if (!building_data_transfer_possible(b)) {
        return 0;
    }

    switch (data_type) {
        case DATA_TYPE_ROADBLOCK:
            b->data.roadblock.exceptions = data.i16;
            break;
        case DATA_TYPE_MARKET:
            memcpy(b->accepted_goods, data.resource, sizeof(unsigned char) * RESOURCE_MAX);
            break;
        case DATA_TYPE_GRANARY:
        case DATA_TYPE_WAREHOUSE:
            building_storage_set_data(b->storage_id, data.storage);
            break;
        case DATA_TYPE_DOCK:
            memcpy(b->accepted_goods, data.resource, sizeof(unsigned char) * RESOURCE_MAX);
            b->data.dock.has_accepted_route_ids = data.i8;
            b->data.dock.accepted_route_ids = data.i32;
            break;
        default:
            return 0;
    }
    city_warning_show(WARNING_DATA_PASTE_SUCCESS, NEW_WARNING_SLOT);
    return 1;

}

building_data_type building_data_transfer_data_type_from_building_type(building_type type)
{
    if (building_type_is_roadblock(type)) {
        return DATA_TYPE_ROADBLOCK;
    }

    switch (type) {
        case BUILDING_DOCK:
            return DATA_TYPE_DOCK;
        case BUILDING_GRANARY:
            return DATA_TYPE_GRANARY;
        case BUILDING_WAREHOUSE:
        case BUILDING_WAREHOUSE_SPACE:
            return DATA_TYPE_WAREHOUSE;
        case BUILDING_MARKET:
            return DATA_TYPE_MARKET;
        default:
            return DATA_TYPE_NOT_SUPPORTED;
    }
}
