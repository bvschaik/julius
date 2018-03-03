#include "cursor.h"

#include "building/construction.h"
#include "game/system.h"

static const cursor ARROW = {
    0, 0,
    "X                               "
    "XX                              "
    "X.X                             "
    "X..X                            "
    "X...X                           "
    "X....X                          "
    "X.....X                         "
    "X......X                        "
    "X.......X                       "
    "X........X                      "
    "X.........X                     "
    "X..........X                    "
    "X......XXXXXX                   "
    "X...X..X                        "
    "X..X X..X                       "
    "X.X  X..X                       "
    "XX    X..X                      "
    "X     X..X                      "
    "       X..X                     "
    "       X..X                     "
    "        XX                      "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
};

static const cursor SWORD = {
    0, 0,
    "XXXXX                           "
    "X...XX                          "
    "X....XX                         "
    "X.....XX                        "
    "XX.....XX                       "
    " XX.....XX                      "
    "  XX.....XX                     "
    "   XX.....XX                    "
    "    XX.....XX                   "
    "     XX.....XX                  "
    "      XX.....XX                 "
    "       XX.....XX                "
    "        XX.....XXXXX            "
    "         XX.....X..X            "
    "          XX...X...X            "
    "           XX.X...XX            "
    "            XX...XXX            "
    "            X...XX.XXX          "
    "            X..XX....X          "
    "            XXXXXX...X          "
    "                 X..XX          "
    "                 XXXX           "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
    "                                "
};

static const cursor SHOVEL = {
    2, 27,
    "                                "
    "                        XX      "
    "                       XXXX     "
    "                      XX.XXX    "
    "                      X...XXX   "
    "                      XX...XX   "
    "                     XXXX.XX    "
    "                    XX.XXXX     "
    "                   XX.XX        "
    "                  XX.XX         "
    "                 XX.XX          "
    "                XX.XX           "
    "               XX.XX            "
    "              XX.XX             "
    "        X    XX.XX              "
    "       XXXX XX.XX               "
    "      X..XXXX.XX                "
    "     X....XX.XX                 "
    "    X....XX.XX                  "
    "   X....X.XXXXX                 "
    "  X....X...X.XX                 "
    "  X...X...X...XX                "
    " X.......X....X                 "
    " X......X....X                  "
    " X..........X                   "
    " X.........X                    "
    " X........X                     "
    "  X.....XX                      "
    "   XXXXX                        "
    "                                "
    "                                "
    "                                "
};

const cursor *input_cursor_data(cursor_shape cursor_id)
{
    switch (cursor_id) {
        case CURSOR_ARROW: return &ARROW;
        case CURSOR_SHOVEL: return &SHOVEL;
        case CURSOR_SWORD: return &SWORD;
        default: return 0;
    }
}

void input_cursor_update(window_id window)
{
    if (window == WINDOW_CITY_MILITARY) {
        system_set_cursor(CURSOR_SWORD);
    } else if (window == WINDOW_CITY && building_construction_type() == BUILDING_CLEAR_LAND) {
        system_set_cursor(CURSOR_SHOVEL);
    } else {
        system_set_cursor(CURSOR_ARROW);
    }
}
