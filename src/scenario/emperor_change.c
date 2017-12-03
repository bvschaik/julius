#include "emperor_change.h"

#include "city/message.h"
#include "core/random.h"
#include "game/time.h"
#include "scenario/data.h"

static struct {
    int game_year;
    int month;
    int state;
} data;

void scenario_emperor_change_init()
{
    data.game_year = scenario.start_year + scenario.emperor_change.year;
    data.month = 1 + (random_byte() & 7);
    data.state = 0;
}

void scenario_emperor_change_process()
{
    if (!scenario.emperor_change.enabled) {
        return;
    }
    if (data.state == 0) {
        if (game_time_year() == data.game_year && game_time_month() == data.month) {
            data.state = 1; // done
            game.messages.post(1, MESSAGE_EMPEROR_CHANGE, 0, 0);
        }
    }
}

void scenario_emperor_change_save_state(buffer *time, buffer *state)
{
    buffer_write_i32(time, data.game_year);
    buffer_write_i32(time, data.month);
    buffer_write_i32(state, data.state);
}

void scenario_emperor_change_load_state(buffer *time, buffer *state)
{
    data.game_year = buffer_read_i32(time);
    data.month = buffer_read_i32(time);
    data.state = buffer_read_i32(state);
}
