#include "bookmark.h"

#include "city/view.h"
#include "map/grid.h"
#include "map/point.h"

#define MAX_BOOKMARKS 4

static map_point bookmarks[MAX_BOOKMARKS];

void map_bookmarks_clear(void)
{
    for (int i = 0; i < MAX_BOOKMARKS; i++) {
        bookmarks[i].x = -1;
        bookmarks[i].y = -1;
    }
}

void map_bookmark_save(int number)
{
    if (number >= 0 && number < MAX_BOOKMARKS) {
        city_view_get_camera_absolute(&bookmarks[number].x, &bookmarks[number].y);
    }
}

int map_bookmark_go_to(int number)
{
    if (number >= 0 && number < MAX_BOOKMARKS) {
        int x = bookmarks[number].x;
        int y = bookmarks[number].y;
        if (x > -1 && map_grid_offset(x, y) > -1) {
            city_view_set_camera_absolute(x, y);
            return 1;
        }
    }
    return 0;
}

void map_bookmark_save_state(buffer *buf)
{
    for (int i = 0; i < MAX_BOOKMARKS; i++) {
        buffer_write_i32(buf, bookmarks[i].x);
        buffer_write_i32(buf, bookmarks[i].y);
    }
}

void map_bookmark_load_state(buffer *buf)
{
    for (int i = 0; i < MAX_BOOKMARKS; i++) {
        bookmarks[i].x = buffer_read_i32(buf);
        bookmarks[i].y = buffer_read_i32(buf);
    }
}
