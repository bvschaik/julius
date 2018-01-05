#include "CityBuildings.h"

#include "../Data/State.h"

#include "city/view.h"
#include "core/direction.h"
#include "sound/city.h"

void UI_CityBuildings_scrollMap(int direction)
{
    if (city_view_scroll(direction)) {
        sound_city_decay_views();
    }
}
