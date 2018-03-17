#include "data.h"

#include "city/constants.h"
#include "city/gods.h"

#include "Data/CityInfo.h"

#include <string.h>

void city_data_init()
{
    memset(&Data_CityInfo, 0, sizeof(struct _Data_CityInfo));

    Data_CityInfo_Extra.unknownBytes[0] = 0;
    Data_CityInfo_Extra.unknownBytes[1] = 0;

    Data_CityInfo.citySentiment = 60;
    Data_CityInfo.healthRateTarget = 50;
    Data_CityInfo.healthRate = 50;
    Data_CityInfo.__unknown_00c0 = 3;
    Data_CityInfo.wagesRome = 30;
    Data_CityInfo.wages = 30;
    Data_CityInfo.taxPercentage = 7;
    Data_CityInfo.tradeNextImportResourceCaravan = 1;
    Data_CityInfo.tradeNextImportResourceCaravanBackup = 1;
    Data_CityInfo.monthlyPopulationNextIndex = 0;
    Data_CityInfo.monthsSinceStart = 0;
    Data_CityInfo.monthsSinceFestival = 1;
    Data_CityInfo.festivalSize = FESTIVAL_SMALL;
    Data_CityInfo.giftCost_modest = 0;
    Data_CityInfo.giftCost_generous = 0;
    Data_CityInfo.giftCost_lavish = 0;

    city_gods_reset();
}

static void save_entry_exit(buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.entryPointFlag.x);
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.entryPointFlag.y);
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.exitPointFlag.x);
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.exitPointFlag.y);
    
    buffer_write_i32(entry_exit_grid_offset, Data_CityInfo_Extra.entryPointFlag.gridOffset);
    buffer_write_i32(entry_exit_grid_offset, Data_CityInfo_Extra.exitPointFlag.gridOffset);
}

static void load_entry_exit(buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    Data_CityInfo_Extra.entryPointFlag.x = buffer_read_i32(entry_exit_xy);
    Data_CityInfo_Extra.entryPointFlag.y = buffer_read_i32(entry_exit_xy);
    Data_CityInfo_Extra.exitPointFlag.x = buffer_read_i32(entry_exit_xy);
    Data_CityInfo_Extra.exitPointFlag.y = buffer_read_i32(entry_exit_xy);
    
    Data_CityInfo_Extra.entryPointFlag.gridOffset = buffer_read_i32(entry_exit_grid_offset);
    Data_CityInfo_Extra.exitPointFlag.gridOffset = buffer_read_i32(entry_exit_grid_offset);
}

void city_data_save_state(buffer *main, buffer *faction, buffer *faction_unknown, buffer *graph_order,
                          buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    buffer_write_raw(main, &Data_CityInfo, 36136);
    
    buffer_write_i32(faction, Data_CityInfo_Extra.ciid);
    buffer_write_i8(faction_unknown, Data_CityInfo_Extra.unknownBytes[0]);
    buffer_write_i8(faction_unknown, Data_CityInfo_Extra.unknownBytes[1]);
    buffer_write_i32(graph_order, Data_CityInfo_Extra.populationGraphOrder);
    buffer_write_i32(graph_order, Data_CityInfo_Extra.unknownOrder);

    save_entry_exit(entry_exit_xy, entry_exit_grid_offset);
}

void city_data_load_state(buffer *main, buffer *faction, buffer *faction_unknown, buffer *graph_order,
                          buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    buffer_read_raw(main, &Data_CityInfo, 36136);

    Data_CityInfo_Extra.ciid = buffer_read_i32(faction);
    Data_CityInfo_Extra.unknownBytes[0] = buffer_read_i8(faction_unknown);
    Data_CityInfo_Extra.unknownBytes[1] = buffer_read_i8(faction_unknown);
    Data_CityInfo_Extra.populationGraphOrder = buffer_read_i32(graph_order);
    Data_CityInfo_Extra.unknownOrder = buffer_read_i32(graph_order);

    load_entry_exit(entry_exit_xy, entry_exit_grid_offset);
}
