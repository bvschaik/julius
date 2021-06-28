#include "data_transfer.h"

#include "building/storage.h"

#include <string.h>

static struct {
    building_data_type data_type;
    int subtype;
    building_storage storage;
    int extended_data[16];
} data;

int building_data_transfer_possible(building *b)
{
    building_data_type data_type = building_data_transfer_data_type_from_building_type(b->type);
    if (data.data_type == DATA_TYPE_NOT_SUPPORTED || data_type == DATA_TYPE_NOT_SUPPORTED) {
        return 0;
    }
    if (data.data_type != data_type) {
        return 0;
    }

    return 1;
}

int building_data_transfer_copy(building *b)
{
    building_data_type data_type = building_data_transfer_data_type_from_building_type(b->type);
    if (data_type != DATA_TYPE_NOT_SUPPORTED) {
        memset(&data, 0, sizeof(data));
        data.data_type = data_type;
    }

    const building_storage *storage;

    switch (data_type) {
        case DATA_TYPE_ROADBLOCK:
            data.subtype = b->subtype.roadblock_exceptions;
            return 1;
        case DATA_TYPE_MARKET:
            data.subtype = b->subtype.market_goods;
            return 1;
        case DATA_TYPE_GRANARY:
            storage = building_storage_get(b->storage_id);
            data.storage = *storage;
            return 1;
        case DATA_TYPE_WAREHOUSE:
            storage = building_storage_get(b->storage_id);
            data.storage = *storage;
            return 1;
        case DATA_TYPE_DOCK:
            data.subtype = b->subtype.market_goods;
            data.extended_data[0] = b->data.dock.has_accepted_route_ids;
            data.extended_data[1] = b->data.dock.accepted_route_ids;
            return 1;
        default:
            return 0;
    }
}

int building_data_transfer_paste(building *b)
{
    building_data_type data_type = building_data_transfer_data_type_from_building_type(b->type);

    if (!building_data_transfer_possible(b)) {
        return 0;
    }

    switch (data_type) {
        case DATA_TYPE_ROADBLOCK:
            b->subtype.roadblock_exceptions = data.subtype;
            return 1;
        case DATA_TYPE_MARKET:
            b->subtype.market_goods = data.subtype;
            return 1;
        case DATA_TYPE_GRANARY:
        case DATA_TYPE_WAREHOUSE:
            building_storage_set_data(b->storage_id, data.storage);
            return 1;
        case DATA_TYPE_DOCK:
            b->subtype.market_goods = data.subtype;
            b->data.dock.has_accepted_route_ids = data.extended_data[0];
            b->data.dock.accepted_route_ids = data.extended_data[1];
            return 1;
        default:
            return 0;
    }
}

building_data_type building_data_transfer_data_type_from_building_type(building_type type)
{
    switch (type) {
        case BUILDING_ROADBLOCK:
            return DATA_TYPE_ROADBLOCK;
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
