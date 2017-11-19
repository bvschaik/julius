#include "Settings.h"

#include "Data/Settings.h"

#include "core/io.h"

void Settings_load()
{
	io_read_file_into_buffer("c3map.inf", &Data_Settings_Map, 48);
}

void Settings_save()
{
	io_write_buffer_to_file("c3map.inf", &Data_Settings_Map, 48);
}
