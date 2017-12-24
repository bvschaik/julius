#include "Cursor.h"

#include "System.h"
#include "UI/Window.h"

#include "building/construction.h"

static const struct Cursor arrow = {
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

static const struct Cursor sword = {
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

static const struct Cursor shovel = {
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

const struct Cursor *Cursor_getData(int cursorId)
{
	switch (cursorId) {
		case 0: return &arrow;
		case 1: return &shovel;
		case 2: return &sword;
		default: return 0;
	}
}

void Cursor_set()
{
	if (UI_Window_getId() == Window_CityMilitary) {
		System_setCursor(Cursor_Sword);
	} else if (UI_Window_getId() == Window_City && building_construction_type() == BUILDING_CLEAR_LAND) {
		System_setCursor(Cursor_Shovel);
	} else {
		System_setCursor(Cursor_Arrow);
	}
}
