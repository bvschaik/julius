#include "gladiator_revolt.h"

#include "building/count.h"
#include "city/games.h"
#include "city/message.h"
#include "core/random.h"
#include "game/time.h"
#include "scenario/data.h"

static struct {
    int game_year;
    int month;
    int end_month;
    int state;
} data;

static int start_revolt(void)
{
    if (building_count_active(BUILDING_GLADIATOR_SCHOOL) > 0 && !city_games_executions_active()) {
        data.state = EVENT_IN_PROGRESS;
        city_message_post(1, MESSAGE_GLADIATOR_REVOLT, 0, 0);
    } else {
        data.state = EVENT_FINISHED;
    }
    return data.state;
}

void scenario_gladiator_revolt_init(void)
{
    data.game_year = scenario.start_year + scenario.gladiator_revolt.year;
    data.month = 3 + (random_byte() & 3);
    data.end_month = 3 + data.month;
    data.state = EVENT_NOT_STARTED;
}

void scenario_gladiator_revolt_process(void)
{
    if (!scenario.gladiator_revolt.enabled) {
        return;
    }

    if (data.state == EVENT_NOT_STARTED) {
        if (game_time_year() == data.game_year && game_time_month() == data.month) {
            start_revolt();
        }
    } else if (data.state == EVENT_IN_PROGRESS) {
        if (data.end_month == game_time_month()) {
            data.state = EVENT_FINISHED;
            city_message_post(1, MESSAGE_GLADIATOR_REVOLT_FINISHED, 0, 0);
        }
    }
}

int scenario_gladiator_revolt_is_in_progress(void)
{
    return data.state == EVENT_IN_PROGRESS;
}

int scenario_gladiator_revolt_is_finished(void)
{
    return data.state == EVENT_FINISHED;
}

void scenario_gladiator_revolt_start_new(void)
{
    if (data.state == EVENT_IN_PROGRESS) {
        return;
    }

    if (!scenario.gladiator_revolt.enabled) {
        scenario.gladiator_revolt.enabled = 1;
    }

    data.game_year = game_time_year();
    data.month = game_time_month();
    data.end_month = 3 + data.month;

    start_revolt();
}

void scenario_gladiator_revolt_save_state(buffer *buf)
{
    buffer_write_i32(buf, data.game_year);
    buffer_write_i32(buf, data.month);
    buffer_write_i32(buf, data.end_month);
    buffer_write_i32(buf, data.state);
}

void scenario_gladiator_revolt_load_state(buffer *buf)
{
    data.game_year = buffer_read_i32(buf);
    data.month = buffer_read_i32(buf);
    data.end_month = buffer_read_i32(buf);
    data.state = buffer_read_i32(buf);
}
