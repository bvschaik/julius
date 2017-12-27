#include "cursor.h"

#include "System.h"

#include "building/construction.h"

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

const cursor *input_cursor_data(int cursor_id)
{
    switch (cursor_id) {
        case CURSOR_ARROW: return &ARROW;
        case CURSOR_SHOVEL: return &SHOVEL;
        case CURSOR_SWORD: return &SWORD;
        default: return 0;
    }
}

void input_cursor_update(WindowId window)
{
    if (window == Window_CityMilitary) {
        System_setCursor(CURSOR_SWORD);
    } else if (window == Window_City && building_construction_type() == BUILDING_CLEAR_LAND) {
        System_setCursor(CURSOR_SHOVEL);
    } else {
        System_setCursor(CURSOR_ARROW);
    }
}
