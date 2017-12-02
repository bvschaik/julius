#include "game_settings.h"

#include <ui>
#include <data>
#include <game>
#include <scenario>

#include "core/io.h"

void Settings_load()
{
    io_read_file_into_buffer("c3map.inf", &Data_State.map, 48);
}

void Settings_save()
{
    io_write_buffer_to_file("c3map.inf", &Data_State.map, 48);
}
