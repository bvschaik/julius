#include "tutorial.h"

#include "building/menu.h"
#include "city/buildings.h"
#include "city/message.h"
#include "city/mission.h"
#include "city/population.h"
#include "city/resource.h"
#include "game/resource.h"
#include "game/time.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

static struct {
    struct {
        int fire;
        int crime;
        int collapse;
        int senate_built;
    } tutorial1;
    struct {
        int granary_built;
        int population_250_reached;
        int population_450_reached;
        int pottery_made;
        int pottery_made_year;
    } tutorial2;
    struct {
        int disease;
    } tutorial3;
} data;

void tutorial_init(void)
{
    int tut1 = 1, tut2 = 1, tut3 = 1;
    if (scenario_is_tutorial_1()) {
        tut1 = tut2 = 0;
    } else if (scenario_is_tutorial_2()) {
        tut2 = 0;
    } else if (scenario_is_tutorial_3()) {
        tut3 = 0;
    }

    data.tutorial1.fire = tut1;
    data.tutorial1.crime = tut1;
    data.tutorial1.collapse = tut1;
    data.tutorial1.senate_built = tut1;
    city_mission_tutorial_set_fire_message_shown(tut1);

    data.tutorial2.granary_built = tut2;
    data.tutorial2.population_250_reached = tut2;
    data.tutorial2.population_450_reached = tut2;
    data.tutorial2.pottery_made = tut2;
    data.tutorial2.pottery_made_year = tut2;

    data.tutorial3.disease = tut3;
    city_mission_tutorial_set_disease_message_shown(tut3);
}

tutorial_availability tutorial_advisor_empire_availability(void)
{
    if (scenario_is_tutorial_1()) {
        return NOT_AVAILABLE;
    } else if (scenario_is_tutorial_2() && !data.tutorial2.population_250_reached) {
        return NOT_AVAILABLE_YET;
    } else {
        return AVAILABLE;
    }
}

tutorial_build_buttons tutorial_get_build_buttons(void)
{
    if (scenario_is_tutorial_1()) {
        if (!data.tutorial1.fire && !data.tutorial1.crime) {
            return TUT1_BUILD_START;
        } else if (!data.tutorial1.collapse) {
            return TUT1_BUILD_AFTER_FIRE;
        } else if (!data.tutorial1.senate_built) {
            return TUT1_BUILD_AFTER_COLLAPSE;
        }
    } else if (scenario_is_tutorial_2()) {
        if (!data.tutorial2.granary_built) {
            return TUT2_BUILD_START;
        } else if (!data.tutorial2.population_250_reached) {
            return TUT2_BUILD_UP_TO_250;
        } else if (!data.tutorial2.population_450_reached) {
            return TUT2_BUILD_UP_TO_450;
        } else if (!data.tutorial2.pottery_made) {
            return TUT2_BUILD_AFTER_450;
        }
    }
    return TUT_BUILD_NORMAL;
}

int tutorial_get_population_cap(int current_cap)
{
    if (scenario_is_tutorial_1()) {
        if (!data.tutorial1.fire ||
            !data.tutorial1.collapse ||
            !data.tutorial1.senate_built) {
            return 80;
        }
    } else if (scenario_is_tutorial_2()) {
        if (!data.tutorial2.granary_built) {
            return 150;
        } else if (!data.tutorial2.pottery_made) {
            return 520;
        }
    }
    return current_cap;
}

int tutorial_get_immediate_goal_text(void)
{
    if (scenario_is_tutorial_1()) {
        if (!data.tutorial1.fire && !data.tutorial1.crime) {
            return 17;
        } else if (!data.tutorial1.collapse) {
            return 18;
        } else if (!data.tutorial1.senate_built) {
            return 19;
        } else {
            return 20;
        }
    } else if (scenario_is_tutorial_2()) {
        if (!data.tutorial2.granary_built) {
            return 21;
        } else if (!data.tutorial2.population_250_reached) {
            return 22;
        } else if (!data.tutorial2.population_450_reached) {
            return 23;
        } else if (!data.tutorial2.pottery_made) {
            return 24;
        } else {
            return 25;
        }
    }
    return 0;
}

int tutorial_adjust_request_year(int *year)
{
    if (scenario_is_tutorial_2()) {
        if (!data.tutorial2.pottery_made) {
            return 0;
        }
        *year = data.tutorial2.pottery_made_year;
    }
    return 1;
}

int tutorial_extra_fire_risk(void)
{
    return !data.tutorial1.fire;
}

int tutorial_extra_damage_risk(void)
{
    return data.tutorial1.fire && !data.tutorial1.collapse;
}

static void post_message(int message)
{
    city_message_post(1, message, 0, 0);
}

int tutorial_handle_fire(void)
{
    if (data.tutorial1.fire) {
        return 0;
    }
    data.tutorial1.fire = 1;
    building_menu_update();
    post_message(MESSAGE_TUTORIAL_FIRE);
    return 1;
}

int tutorial_handle_collapse(void)
{
    if (data.tutorial1.collapse) {
        return 0;
    }
    data.tutorial1.collapse = 1;
    building_menu_update();
    post_message(MESSAGE_TUTORIAL_COLLAPSE);
    return 1;
}

void tutorial_on_crime(void)
{
    if (!data.tutorial1.crime) {
        data.tutorial1.crime = 1;
        building_menu_update();
    }
}

void tutorial_on_disease(void)
{
    data.tutorial3.disease = 1;
}

void tutorial_on_filled_granary(void)
{
    if (!data.tutorial2.granary_built) {
        data.tutorial2.granary_built = 1;
        building_menu_update();
        post_message(MESSAGE_TUTORIAL_WATER);
    }
}

void tutorial_on_add_to_warehouse(void)
{
    if (!data.tutorial2.pottery_made && city_resource_count(RESOURCE_POTTERY) >= 1) {
        data.tutorial2.pottery_made = 1;
        data.tutorial2.pottery_made_year = game_time_year();
        building_menu_update();
        post_message(MESSAGE_TUTORIAL_TRADE);
    }
}

void tutorial_on_day_tick(void)
{
    if (data.tutorial1.fire) {
        city_mission_tutorial_set_fire_message_shown(1);
    }
    if (data.tutorial3.disease && city_mission_tutorial_show_disease_message()) {
        post_message(MESSAGE_TUTORIAL_HEALTH);
    }
    if (data.tutorial2.granary_built) {
        if (!data.tutorial2.population_250_reached && city_population() >= 250) {
            data.tutorial2.population_250_reached = 1;
            building_menu_update();
            post_message(MESSAGE_TUTORIAL_GROWING_YOUR_CITY);
        }
    }
    if (data.tutorial2.population_250_reached) {
        if (!data.tutorial2.population_450_reached && city_population() >= 450) {
            data.tutorial2.population_450_reached = 1;
            building_menu_update();
            post_message(MESSAGE_TUTORIAL_TAXES_INDUSTRY);
        }
    }
    if (data.tutorial1.fire && !data.tutorial1.senate_built) {
        int population_almost = city_population() >= scenario_criteria_population() - 20;
        if (!game_time_day() || population_almost) {
            if (city_buildings_has_senate()) {
                city_mission_tutorial_add_senate();
            }
            if (city_mission_tutorial_has_senate() || population_almost) {
                data.tutorial1.senate_built = 1;
                building_menu_update();
                post_message(MESSAGE_TUTORIAL_RELIGION);
            }
        }
    }
}

void tutorial_on_month_tick(void)
{
    if (scenario_is_tutorial_3()) {
        if (game_time_month() == 5) {
            city_message_post_with_message_delay(MESSAGE_CAT_TUTORIAL3, 1, MESSAGE_TUTORIAL_HUNGER_HALTS_IMMIGRANTS, 1200);
        }
    }
}

void tutorial_save_state(buffer *buf1, buffer *buf2, buffer *buf3)
{
    buffer_write_i32(buf1, data.tutorial1.fire);
    buffer_write_i32(buf1, data.tutorial1.crime);
    buffer_write_i32(buf1, data.tutorial1.collapse);
    buffer_write_i32(buf1, data.tutorial2.granary_built);
    buffer_write_i32(buf1, data.tutorial2.population_250_reached);
    buffer_write_i32(buf1, data.tutorial1.senate_built);
    buffer_write_i32(buf1, data.tutorial2.population_450_reached);
    buffer_write_i32(buf1, data.tutorial2.pottery_made);

    buffer_write_i32(buf2, data.tutorial2.pottery_made_year);

    buffer_write_i32(buf3, data.tutorial3.disease);
}


void tutorial_load_state(buffer *buf1, buffer *buf2, buffer *buf3)
{
    data.tutorial1.fire = buffer_read_i32(buf1);
    data.tutorial1.crime = buffer_read_i32(buf1);
    data.tutorial1.collapse = buffer_read_i32(buf1);
    data.tutorial2.granary_built = buffer_read_i32(buf1);
    data.tutorial2.population_250_reached = buffer_read_i32(buf1);
    data.tutorial1.senate_built = buffer_read_i32(buf1);
    data.tutorial2.population_450_reached = buffer_read_i32(buf1);
    data.tutorial2.pottery_made = buffer_read_i32(buf1);

    data.tutorial2.pottery_made_year = buffer_read_i32(buf2);

    data.tutorial3.disease = buffer_read_i32(buf3);
}
