#include "gods.h"

#include "building/count.h"
#include "building/granary.h"
#include "building/industry.h"
#include "city/culture.h"
#include "city/data_private.h"
#include "city/health.h"
#include "city/message.h"
#include "city/population.h"
#include "city/sentiment.h"
#include "city/trade.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/random.h"
#include "figure/formation_legion.h"
#include "figuretype/water.h"
#include "game/settings.h"
#include "game/time.h"
#include "scenario/property.h"
#include "scenario/invasion.h"

#define TIE 10

#define FLAT_CHANCE_FOR_BLESSING 1
#define HAPPINESS_BLESSING_FACTOR 20
#define FESTIVAL_BLESSING_FACTOR 6
#define FESTIVAL_BLESSING_LENGTH 24
#define BLESSING_BOLTS_NEEDED_FOR_BLESSING 5

void city_gods_reset(void)
{
    for (int i = 0; i < MAX_GODS; i++) {
        god_status *god = &city_data.religion.gods[i];
        god->target_happiness = 50;
        god->happiness = 50;
        god->wrath_bolts = 0;
        god->blessing_done = 0;
        god->small_curse_done = 0;
        god->happy_bolts = 0;
        god->months_since_festival = 0;
    }
    city_data.religion.angry_message_delay = 0;
}

void city_gods_reset_neptune_blessing(void)
{
    city_data.religion.neptune_double_trade_active = 0;
}

void city_gods_update_blessings(void)
{
    if (city_data.religion.neptune_double_trade_active > 0) {
        city_data.religion.neptune_double_trade_active--;
    }

    if (city_data.religion.venus_blessing_months_left > 0) {
        city_data.religion.venus_blessing_months_left--;
    }
}

static void perform_blessing(god_type god)
{
    switch (god) {
        case GOD_CERES:
            city_message_post(1, MESSAGE_BLESSING_FROM_CERES, 0, 0);
            building_bless_farms();
            break;
        case GOD_NEPTUNE:
            city_message_post(1, MESSAGE_BLESSING_FROM_NEPTUNE_ALTERNATE, 0, 0);
            city_data.religion.neptune_double_trade_active = NEPTUNE_BLESSING_MONTHS;
            break;
        case GOD_MERCURY:
            city_message_post(1, MESSAGE_BLESSING_FROM_MERCURY_ALTERNATE, 0, 0);
            building_bless_industry();
            break;
        case GOD_MARS:
            city_message_post(1, MESSAGE_BLESSING_FROM_MARS, 0, 0);
            city_data.religion.mars_spirit_power = 10;
            break;
        case GOD_VENUS:
            city_message_post(1, MESSAGE_BLESSING_FROM_VENUS_ALTERNATE, 0, 0);
            city_data.sentiment.blessing_festival_boost += 18;
            city_population_venus_blessing();
            city_data.religion.venus_blessing_months_left = VENUS_BLESSING_MONTHS;
            break;
    }
}

static void perform_small_curse(god_type god)
{
    switch (god) {
        case GOD_CERES:
            city_message_post(1, MESSAGE_CERES_IS_UPSET, 0, 0);
            building_curse_farms(0);
            break;
        case GOD_NEPTUNE:
            city_message_post(1, MESSAGE_NEPTUNE_IS_UPSET, 0, 0);
            figure_sink_all_ships();
            city_data.religion.neptune_sank_ships = 1;
            break;
        case GOD_MERCURY:
            city_message_post(1, MESSAGE_MERCURY_IS_UPSET, 0, 0);
            building_granary_warehouse_curse(0);
            break;
        case GOD_MARS:
            if (scenario_invasion_start_from_mars()) {
                city_message_post(1, MESSAGE_MARS_IS_UPSET, 0, 0);
            } else {
                city_message_post(1, MESSAGE_WRATH_OF_MARS_NO_NATIVES, 0, 0);
            }
            break;
        case GOD_VENUS:
            city_message_post(1, MESSAGE_VENUS_IS_UPSET, 0, 0);
            city_data.sentiment.blessing_festival_boost -= 15;
            city_health_change(-10);
            city_sentiment_update();
            break;
    }
}

static int perform_large_curse(god_type god)
{
    switch (god) {
        case GOD_CERES:
            city_message_post(1, MESSAGE_WRATH_OF_CERES, 0, 0);
            building_curse_farms(1);
            break;
        case GOD_NEPTUNE:
            if (city_data.trade.num_sea_routes <= 0) {
                city_message_post(1, MESSAGE_WRATH_OF_NEPTUNE_NO_SEA_TRADE, 0, 0);
                return 0;
            } else {
                city_message_post(1, MESSAGE_WRATH_OF_NEPTUNE, 0, 0);
                figure_sink_all_ships();
                city_data.religion.neptune_sank_ships = 1;
                city_trade_start_sea_trade_problems(80);
            }
            break;
        case GOD_MERCURY:
            city_message_post(1, MESSAGE_WRATH_OF_MERCURY, 0, 0);
            building_granary_warehouse_curse(1);
            break;
        case GOD_MARS:
            if (formation_legion_curse()) {
                city_message_post(1, MESSAGE_WRATH_OF_MARS, 0, 0);
                scenario_invasion_start_from_mars();
            } else {
                city_message_post(1, MESSAGE_WRATH_OF_MARS_NO_MILITARY, 0, 0);
            }
            break;
        case GOD_VENUS:
            city_message_post(1, MESSAGE_WRATH_OF_VENUS, 0, 0);
            city_sentiment_set_max_happiness(40);
            city_sentiment_change_happiness(-10);
            if (city_data.health.value >= 80) {
                city_health_change(-50);
            } else if (city_data.health.value >= 60) {
                city_health_change(-40);
            } else {
                city_health_change(-20);
            }
            city_data.religion.venus_curse_active = 1;
            city_sentiment_update();
            break;
    }
    return 1;
}

static void update_god_moods(void)
{
    for (int i = 0; i < MAX_GODS; i++) {
        god_status *god = &city_data.religion.gods[i];
        if (god->happiness < god->target_happiness) {
            god->happiness++;
        } else if (god->happiness > god->target_happiness) {
            god->happiness--;
        }
        if (scenario_is_tutorial_1()) {
            if (god->happiness < 50) {
                god->happiness = 50;
            }
        }
        if (god->happiness > 50) {
            god->small_curse_done = 0;
        }
        if (god->happiness < 50) {
            god->blessing_done = 0;
        }
    }

    int god_id = random_byte() & 7;
    if (god_id < MAX_GODS) {
        god_status *god = &city_data.religion.gods[god_id];
        if (god->happiness >= 50) {
            god->wrath_bolts = 0;
        } else if (god->happiness < 40) {
            if (god->happy_bolts > 0) {
                god->happy_bolts -= 1;
            } else if (god->happiness >= 20) {
                god->wrath_bolts += 1;
            } else if (god->happiness >= 10) {
                god->wrath_bolts += 2;
            } else {
                god->wrath_bolts += 5;
            }
        }
        if (god->wrath_bolts > 50) {
            god->wrath_bolts = 50;
        }
        if (god->happiness >= 50) {
            int chance_for_happy_bolt = (god->happiness - 50) / HAPPINESS_BLESSING_FACTOR + FLAT_CHANCE_FOR_BLESSING;
            if (god->months_since_festival <= FESTIVAL_BLESSING_LENGTH) {
                chance_for_happy_bolt += (FESTIVAL_BLESSING_LENGTH - god->months_since_festival) / FESTIVAL_BLESSING_FACTOR + FLAT_CHANCE_FOR_BLESSING;
            }
            random_generate_next();
            int roll = random_short_alt() % 100;
            if (roll < chance_for_happy_bolt) {
                god->happy_bolts++;
            }
            if (god->happy_bolts > 5) {
                god->happy_bolts = 5;
            }
        }
    }
    if (game_time_day() != 0) {
        return;
    }

    // handle blessings, curses, etc every month
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].months_since_festival++;
    }
    if (god_id >= MAX_GODS) {
        if (city_gods_calculate_least_happy()) {
            god_id = city_data.religion.least_happy_god - 1;
        }
    }
    if (!setting_gods_enabled()) {
        return;
    }

    if (god_id < MAX_GODS) {
        god_status *god = &city_data.religion.gods[god_id];
        if (god->happiness >= 50 && god->happy_bolts >= 5) {
            god->blessing_done = 1;
            god->happy_bolts = 0;
            perform_blessing(god_id);
        } else if (god->wrath_bolts >= 20 && !god->small_curse_done && god->months_since_festival > 3) {
            god->small_curse_done = 1;
            god->wrath_bolts = 0;
            god->happiness += 12;
            perform_small_curse(god_id);
        } else if (god->wrath_bolts >= 50 && god->months_since_festival > 3) {
            if (scenario_campaign_rank() < 4 && !scenario_is_custom()) {
                // no large curses in early scenarios
                god->small_curse_done = 0;
                return;
            }
            god->wrath_bolts = 0;
            god->happiness += 30;
            if (!perform_large_curse(god_id)) {
                return;
            }
        }
    }

    int min_happiness = 100;
    for (int i = 0; i < MAX_GODS; i++) {
        if (city_data.religion.gods[i].happiness < min_happiness) {
            min_happiness = city_data.religion.gods[i].happiness;
        }
    }
    if (city_data.religion.angry_message_delay) {
        city_data.religion.angry_message_delay--;
    } else if (min_happiness < 30) {
        city_data.religion.angry_message_delay = 20;
        if (min_happiness < 10) {
            city_message_post(0, MESSAGE_GODS_WRATHFUL, 0, 0);
        } else {
            city_message_post(0, MESSAGE_GODS_UNHAPPY, 0, 0);
        }
    }
}

void city_gods_calculate_moods(int update_moods)
{
    // base happiness: percentage of houses covered
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].target_happiness = city_culture_coverage_religion(i);
    }

    int max_temples = 0;
    int max_god = TIE;
    int min_temples = 100000;
    int min_god = TIE;
    for (int i = 0; i < MAX_GODS; i++) {
        int num_temples = 0;
        switch (i) {
            case GOD_CERES:
                num_temples = building_count_total(BUILDING_SMALL_TEMPLE_CERES) + building_count_total(BUILDING_LARGE_TEMPLE_CERES) + building_count_total(BUILDING_GRAND_TEMPLE_CERES);
                break;
            case GOD_NEPTUNE:
                num_temples = building_count_total(BUILDING_SMALL_TEMPLE_NEPTUNE) + building_count_total(BUILDING_LARGE_TEMPLE_NEPTUNE) + building_count_total(BUILDING_GRAND_TEMPLE_NEPTUNE);
                break;
            case GOD_MERCURY:
                num_temples = building_count_total(BUILDING_SMALL_TEMPLE_MERCURY) + building_count_total(BUILDING_LARGE_TEMPLE_MERCURY) + building_count_total(BUILDING_GRAND_TEMPLE_MERCURY);
                break;
            case GOD_MARS:
                num_temples = building_count_total(BUILDING_SMALL_TEMPLE_MARS) + building_count_total(BUILDING_LARGE_TEMPLE_MARS) + building_count_total(BUILDING_GRAND_TEMPLE_MARS);
                break;
            case GOD_VENUS:
                num_temples = building_count_total(BUILDING_SMALL_TEMPLE_VENUS) + building_count_total(BUILDING_LARGE_TEMPLE_VENUS) + building_count_total(BUILDING_GRAND_TEMPLE_VENUS);
                break;
        }
        if (num_temples == max_temples) {
            max_god = TIE;
        } else if (num_temples > max_temples) {
            max_temples = num_temples;
            max_god = i;
        }
        if (num_temples == min_temples) {
            min_god = TIE;
        } else if (num_temples < min_temples) {
            min_temples = num_temples;
            min_god = i;
        }
    }
    // happiness factor based on months since festival (max 40)
    for (int i = 0; i < MAX_GODS; i++) {
        int festival_penalty = city_data.religion.gods[i].months_since_festival;
        if (festival_penalty > 40) {
            festival_penalty = 40;
        }
        city_data.religion.gods[i].target_happiness += 12 - festival_penalty;
    }

    if (!(config_get(CONFIG_GP_CH_JEALOUS_GODS))) {
        if (max_god < MAX_GODS) {
            if (city_data.religion.gods[max_god].target_happiness >= 50) {
                city_data.religion.gods[max_god].target_happiness = 100;
            } else {
                city_data.religion.gods[max_god].target_happiness += 50;
            }
        }
        if (min_god < MAX_GODS) {
            city_data.religion.gods[min_god].target_happiness -= 25;
        }
    }
    int min_happiness;
    if (city_data.population.population < 100) {
        min_happiness = 50;
    } else if (city_data.population.population < 200) {
        min_happiness = 40;
    } else if (city_data.population.population < 300) {
        min_happiness = 30;
    } else if (city_data.population.population < 400) {
        min_happiness = 20;
    } else if (city_data.population.population < 500) {
        min_happiness = 10;
    } else {
        min_happiness = 0;
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].target_happiness =
            calc_bound(city_data.religion.gods[i].target_happiness, min_happiness, 100);
    }
    if (update_moods) {
        update_god_moods();
    }
}

int city_gods_calculate_least_happy(void)
{
    int max_god = 0;
    int max_wrath = 0;
    for (int i = 0; i < MAX_GODS; i++) {
        if (city_data.religion.gods[i].wrath_bolts > max_wrath) {
            max_god = i + 1;
            max_wrath = city_data.religion.gods[i].wrath_bolts;
        }
    }
    if (max_god > 0) {
        city_data.religion.least_happy_god = max_god;
        return 1;
    }
    int min_happiness = 40;
    for (int i = 0; i < MAX_GODS; i++) {
        if (city_data.religion.gods[i].happiness < min_happiness) {
            max_god = i + 1;
            min_happiness = city_data.religion.gods[i].happiness;
        }
    }
    city_data.religion.least_happy_god = max_god;
    return max_god > 0;
}

int city_god_happiness(int god_id)
{
    return city_data.religion.gods[god_id].happiness;
}

int city_god_wrath_bolts(int god_id)
{
    return city_data.religion.gods[god_id].wrath_bolts;
}

int city_god_happy_bolts(int god_id)
{
    return city_data.religion.gods[god_id].happy_bolts;
}

int city_god_months_since_festival(int god_id)
{
    return city_data.religion.gods[god_id].months_since_festival;
}

int city_god_least_happy(void)
{
    return city_data.religion.least_happy_god - 1;
}

int city_god_spirit_of_mars_power(void)
{
    return city_data.religion.mars_spirit_power;
}

void city_god_spirit_of_mars_mark_used(void)
{
    city_data.religion.mars_spirit_power = 0;
}

int city_god_neptune_create_shipwreck_flotsam(void)
{
    if (city_data.religion.neptune_sank_ships) {
        city_data.religion.neptune_sank_ships = 0;
        return 1;
    } else {
        return 0;
    }
}

int city_god_venus_bonus_employment(void)
{
    if (city_data.religion.venus_blessing_months_left > 0) {
        return ((city_data.religion.venus_blessing_months_left / 12) + 1);
    } else {
        return 0;
    }
}

void city_god_blessing_cheat(int god_id)
{
    perform_blessing(god_id);
}
