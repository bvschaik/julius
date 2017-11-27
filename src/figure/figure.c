#include "figure/figure.h"

struct Data_Figure *figure_get(int id)
{
    return &Data_Figures[id];
}
