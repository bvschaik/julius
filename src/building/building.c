#include "building.h"

struct Data_Building *building_get(int id)
{
    return &Data_Buildings[id];
}
