#include "empire.h"
#include "playermessage.h"

#include <data>
#include "building/count.h"
#include "core/buffer.h"
#include "core/calc.h"
#include "core/io.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/time.h"
#include "graphics/image.h"

#include <string.h>

static struct
{
    short initialScrollX;
    short initialScrollY;
    short inUse;
    short __padding[13];
} Data_Empire_Index[40];

void Empire_load(int isCustomScenario, int empireId)
{
    const char *filename = isCustomScenario ? "c32.emp" : "c3.emp";
    io_read_file_part_into_buffer(filename, Data_Empire_Index, 1280, 0);
    int offset = 1280 + 12800 * Data_Scenario.empireId;
    char obj_buffer[12800];
    io_read_file_part_into_buffer(filename, obj_buffer, 12800, offset);
    buffer buf;
    buffer_init(&buf, obj_buffer, 12800);
    empire_object_load(&buf);
}

void Empire_initScroll()
{
    empire_init_scroll(
        Data_Empire_Index[Data_Scenario.empireId].initialScrollX,
        Data_Empire_Index[Data_Scenario.empireId].initialScrollY
    );
}

void Empire_initCities()
{
    empire_object_init_cities();
}

void Empire_initTradeAmountCodes()
{
    empire_object_init_trade_amounts();
}
