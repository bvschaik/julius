#include "service.h"

#include "building/building.h"
#include "building/distribution.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "core/config.h"
#include "core/random.h"
#include "figure/roamer_preview.h"
#include "figuretype/crime.h"
#include "game/resource.h"
#include "game/time.h"
#include "map/building.h"
#include "map/grid.h"

#define MAX_COVERAGE 96
#define TOURISM_COOLDOWN 96

static int provide_culture(int x, int y, void (*callback)(building *))
{
    int serviced = 0;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                if (b->house_size && b->house_population > 0) {
                    callback(b);
                    serviced++;
                }
            }
        }
    }
    return serviced;
}

static void provide_sickness(int x, int y, void (*callback)(building *, int sickness_dest), int sickness_dest)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                random_generate_next();
                // 1/16 chance of spreading sickness
                if (b->house_size && b->house_population > 0 && !(random_short() & 0xf)) {
                    callback(b, sickness_dest);
                }
            }
        }
    }
}

static int provide_entertainment(int x, int y, int shows, void (*callback)(building *, int))
{
    int serviced = 0;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                if (b->house_size && b->house_population > 0) {
                    callback(b, shows);
                    serviced++;
                }
            }
        }
    }
    return serviced;
}

static void labor_seeker_coverage(building *b)
{}

static void theater_coverage(building *b)
{
    b->data.house.theater = MAX_COVERAGE;
}

static void amphitheater_coverage(building *b, int shows)
{
    b->data.house.amphitheater_actor = MAX_COVERAGE;
    if (shows == 2) {
        b->data.house.amphitheater_gladiator = MAX_COVERAGE;
    }
}

static void colosseum_coverage(building *b, int shows)
{
    b->data.house.colosseum_gladiator = MAX_COVERAGE;
    if (shows == 2) {
        b->data.house.colosseum_lion = MAX_COVERAGE;
    }
}

static void arena_coverage(building *b, int shows)
{
    b->house_arena_gladiator = MAX_COVERAGE;
    if (shows == 2) {
        b->house_arena_lion = MAX_COVERAGE;
    }
}

static void hippodrome_coverage(building *b)
{
    b->data.house.hippodrome = MAX_COVERAGE;
}

static void tavern_coverage(building *b, int products)
{
    if (products) {
        b->house_tavern_wine_access = MAX_COVERAGE;
        if (products > 1) {
            b->house_tavern_meat_access = MAX_COVERAGE;
        }
    }
}

static void bathhouse_coverage(building *b)
{
    b->data.house.bathhouse = MAX_COVERAGE;
}

static void religion_coverage_ceres(building *b)
{
    b->data.house.temple_ceres = MAX_COVERAGE;
}

static void religion_coverage_neptune(building *b)
{
    b->data.house.temple_neptune = MAX_COVERAGE;
}

static void religion_coverage_mercury(building *b)
{
    b->data.house.temple_mercury = MAX_COVERAGE;
}

static void religion_coverage_mars(building *b)
{
    b->data.house.temple_mars = MAX_COVERAGE;
}

static void religion_coverage_venus(building *b)
{
    b->data.house.temple_venus = MAX_COVERAGE;
}

static void religion_coverage_pantheon(building *b)
{
    b->house_pantheon_access = MAX_COVERAGE;
}

static void school_coverage(building *b)
{
    b->data.house.school = MAX_COVERAGE;
}

static void academy_coverage(building *b)
{
    b->data.house.academy = MAX_COVERAGE;
}

static void library_coverage(building *b)
{
    b->data.house.library = MAX_COVERAGE;
}

static void barber_coverage(building *b)
{
    b->data.house.barber = MAX_COVERAGE;
}

static void clinic_coverage(building *b)
{
    b->data.house.clinic = MAX_COVERAGE;
}

static void hospital_coverage(building *b)
{
    b->data.house.hospital = MAX_COVERAGE;
}

static void cart_pusher_sickness(building *b, int sickness_dest)
{
    if (!b->sickness_level) {
        b->sickness_level = 1 + (sickness_dest / 10);
    }
}

static int provide_missionary_coverage(int x, int y)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 4, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int building_id = map_building_at(map_grid_offset(xx, yy));
            if (building_id) {
                building *b = building_get(building_id);
                if (b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING) {
                    b->sentiment.native_anger = 0;
                }
            }
        }
    }
    return 1;
}

static int tourist_visit(int x, int y, figure *f, void (*callback)(building *, figure *))
{
    int serviced = 0;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                callback(b, f);
            }
        }
    }
    return serviced;
}

static void tourist_spend(building *b, figure *f)
{
    int can_pay = 0;
    if (!b->is_tourism_venue || b->tourism_disabled) {
        return;
    }

    if (b->type == BUILDING_HIPPODROME) {
        b = building_main(b);
    }
    for (int i = 0; i <= 12; ++i) {
        if (f->tourist.visited_building_type_ids[i]) {
            if (f->tourist.visited_building_type_ids[i] == b->type) {
                if (f->tourist.ticks_since_last_visited_id[i] >= TOURISM_COOLDOWN) {
                    can_pay = 1;
                    f->tourist.ticks_since_last_visited_id[i] = 0;
                }
                break;
            }
        } else {
            f->tourist.visited_building_type_ids[i] = b->type;
            can_pay = 1;
            break;
        }
    }

    if (can_pay) {
        int amount = b->tourism_income;
        f->tourist.tourist_money_spent += amount;
        b->tourism_income_this_year += amount;
        city_finance_treasury_add_tourism(amount);
    }
}

static int provide_service(int x, int y, int *data, void (*callback)(building *, int *))
{
    int serviced = 0;
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                callback(b, data);
                if (b->house_size && b->house_population > 0) {
                    serviced++;
                }
            }
        }
    }
    return serviced;
}

static void engineer_coverage(building *b, int *max_damage_seen)
{
    if (b->type == BUILDING_HIPPODROME) {
        b = building_main(b);
    }
    if (b->damage_risk > *max_damage_seen) {
        *max_damage_seen = b->damage_risk;
    }
    b->damage_risk = 0;
}

static void prefect_coverage(building *b, int *min_happiness_seen)
{
    if (b->type == BUILDING_HIPPODROME) {
        b = building_main(b);
    }
    b->fire_risk = 0;
    if (b->sentiment.house_happiness < *min_happiness_seen) {
        *min_happiness_seen = b->sentiment.house_happiness;
    }
}

static void tax_collector_coverage(building *b, int *max_tax_multiplier)
{
    if (b->house_size && b->house_population > 0) {
        int tax_multiplier = model_get_house(b->subtype.house_level)->tax_multiplier;
        if (tax_multiplier > *max_tax_multiplier) {
            *max_tax_multiplier = tax_multiplier;
        }
        b->house_tax_coverage = 50;
    }
}

static void distribute_good(building *b, building *market, int stock_wanted, int inventory_resource)
{
    if (!building_distribution_is_good_accepted(inventory_resource, market)) {
        return;
    }
    int amount_wanted = stock_wanted - b->data.house.inventory[inventory_resource];
    if (market->data.market.inventory[inventory_resource] > 0 && amount_wanted > 0) {
        if (amount_wanted <= market->data.market.inventory[inventory_resource]) {
            b->data.house.inventory[inventory_resource] += amount_wanted;
            market->data.market.inventory[inventory_resource] -= amount_wanted;
        } else {
            b->data.house.inventory[inventory_resource] += market->data.market.inventory[inventory_resource];
            market->data.market.inventory[inventory_resource] = 0;
        }
    }
}

static void collect_offerings_from_house(building *house, building *temple)
{
    // offerings are generated, not removed from house stores    
    if (house->days_since_offering >= MARS_OFFERING_FREQUENCY) {
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            if (house->data.house.inventory[i]) {
                if (house->house_size == 1 && house->house_is_merged) {
                    temple->data.market.inventory[i] += 2;
                } else {
                    temple->data.market.inventory[i] += house->house_size;
                }
            }
            if (temple->data.market.inventory[i] > 400) {
                temple->data.market.inventory[i] = 400;
            }
        }
        house->days_since_offering = 0;
    }
}

static void distribute_market_resources(building *b, building *market)
{
    int level = b->subtype.house_level;
    if (level < HOUSE_LUXURY_PALACE) {
        level++;
    }
    int max_food_stocks = 4 * b->house_highest_population;
    int food_types_stored_max = 0;
    for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
        if (b->data.house.inventory[i] >= max_food_stocks) {
            food_types_stored_max++;
        }
    }
    const model_house *model = model_get_house(level);
    if (model->food_types) {
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            if (b->data.house.inventory[i] >= max_food_stocks ||
                !building_distribution_is_good_accepted(i, market)) {
                continue;
            }
            if (market->data.market.inventory[i] >= max_food_stocks) {
                b->data.house.inventory[i] += max_food_stocks;
                market->data.market.inventory[i] -= max_food_stocks;
                break;
            } else if (market->data.market.inventory[i]) {
                b->data.house.inventory[i] += market->data.market.inventory[i];
                market->data.market.inventory[i] = 0;
                break;
            }
        }
    }

    int goods_no = 8;

    // Venus base stockpile bonus
    if (building_monument_working(BUILDING_GRAND_TEMPLE_VENUS)) {
        goods_no = 12;
    }

    if (model->pottery) {
        market->data.market.pottery_demand = 10;
        distribute_good(b, market, goods_no * model->pottery, INVENTORY_POTTERY);
    }
    if (model->furniture) {
        market->data.market.furniture_demand = 10;
        distribute_good(b, market, goods_no * model->furniture, INVENTORY_FURNITURE);
    }
    if (model->oil) {
        market->data.market.oil_demand = 10;
        distribute_good(b, market, goods_no * model->oil, INVENTORY_OIL);
    }
    if (model->wine) {
        market->data.market.wine_demand = 10;
        distribute_good(b, market, goods_no * model->wine, INVENTORY_WINE);
    }
}

static int provide_market_goods(int market_building_id, int x, int y)
{
    int serviced = 0;
    building *market = building_get(market_building_id);
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                if (b->house_size && b->house_population > 0) {
                    distribute_market_resources(b, market);
                    serviced++;
                }

            }
        }
    }
    return serviced;
}

static int provide_venus_wine_to_taverns(int market_building_id, int x, int y)
{
    int serviced = 0;
    building *market = building_get(market_building_id);
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                if (b->type == BUILDING_TAVERN) {
                    int amount_wanted = 200 - b->data.market.inventory[INVENTORY_WINE];
                    if (market->data.market.inventory[INVENTORY_WINE] > 0 && amount_wanted > 0) {
                        if (amount_wanted <= market->data.market.inventory[INVENTORY_WINE]) {
                            b->data.market.inventory[INVENTORY_WINE] += amount_wanted;
                            market->data.market.inventory[INVENTORY_WINE] -= amount_wanted;
                        } else {
                            b->data.market.inventory[INVENTORY_WINE] += market->data.market.inventory[INVENTORY_WINE];
                            market->data.market.inventory[INVENTORY_WINE] = 0;
                        }
                    }
                    serviced++;
                }
            }
        }
    }
    return serviced;
}

static int collect_offerings(int market_building_id, int x, int y)
{
    int serviced = 0;
    building *market = building_get(market_building_id);
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                if (b->house_size && b->house_population > 0) {
                    collect_offerings_from_house(b, market);
                    serviced++;
                }
            }
        }
    }
    return serviced;
}

static building *get_entertainment_building(const figure *f)
{
    if (f->action_state == FIGURE_ACTION_94_ENTERTAINER_ROAMING ||
        f->action_state == FIGURE_ACTION_95_ENTERTAINER_RETURNING) {
        return building_get(f->building_id);
    } else { // going to venue
        return building_get(f->destination_building_id);
    }
}

int figure_service_provide_coverage(figure *f)
{
    int houses_serviced = 0;
    int x = f->x;
    int y = f->y;
    building *b;
    switch (f->type) {
        case FIGURE_PATRICIAN:
            return 0;
        case FIGURE_LABOR_SEEKER:
            houses_serviced = provide_culture(x, y, labor_seeker_coverage);
            break;
        case FIGURE_TAX_COLLECTOR:
            {
                int max_tax_rate = 0;
                houses_serviced = provide_service(x, y, &max_tax_rate, tax_collector_coverage);
                f->min_max_seen = max_tax_rate;
                break;
            }
        case FIGURE_MARKET_TRADER:
            houses_serviced = provide_market_goods(f->building_id, x, y);
            break;
        case FIGURE_MARKET_SUPPLIER:
            if (!config_get(CONFIG_GP_CH_NO_SUPPLIER_DISTRIBUTION)) {
                houses_serviced = provide_market_goods(f->building_id, x, y);
            }
            break;
        case FIGURE_BATHHOUSE_WORKER:
            houses_serviced = provide_culture(x, y, bathhouse_coverage);
            break;
        case FIGURE_SCHOOL_CHILD:
            houses_serviced = provide_culture(x, y, school_coverage);
            break;
        case FIGURE_TEACHER:
            houses_serviced = provide_culture(x, y, academy_coverage);
            break;
        case FIGURE_LIBRARIAN:
            houses_serviced = provide_culture(x, y, library_coverage);
            break;
        case FIGURE_BARBER:
            houses_serviced = provide_culture(x, y, barber_coverage);
            break;
        case FIGURE_DOCTOR:
            houses_serviced = provide_culture(x, y, clinic_coverage);
            break;
        case FIGURE_SURGEON:
            houses_serviced = provide_culture(x, y, hospital_coverage);
            break;
        case FIGURE_WAREHOUSEMAN:
        case FIGURE_DOCKER:
        case FIGURE_CART_PUSHER:
            b = building_get(f->building_id);
            building *dest_b = building_get(f->destination_building_id);

            if (b->sickness_level || dest_b->sickness_level) {
                provide_sickness(x, y, cart_pusher_sickness, dest_b->sickness_level);
            }
            break;
        case FIGURE_MISSIONARY:
            houses_serviced = provide_missionary_coverage(x, y);
            break;
        case FIGURE_PRIEST:
            switch (building_get(f->building_id)->type) {
                case BUILDING_SMALL_TEMPLE_CERES:
                case BUILDING_LARGE_TEMPLE_CERES:
                    houses_serviced = provide_culture(x, y, religion_coverage_ceres);
                    provide_market_goods(f->building_id, x, y);
                    break;
                case BUILDING_GRAND_TEMPLE_CERES:
                    houses_serviced = provide_culture(x, y, religion_coverage_ceres);
                    break;
                case BUILDING_SMALL_TEMPLE_NEPTUNE:
                case BUILDING_LARGE_TEMPLE_NEPTUNE:
                case BUILDING_GRAND_TEMPLE_NEPTUNE:
                    houses_serviced = provide_culture(x, y, religion_coverage_neptune);
                    break;
                case BUILDING_SMALL_TEMPLE_MERCURY:
                case BUILDING_LARGE_TEMPLE_MERCURY:
                case BUILDING_GRAND_TEMPLE_MERCURY:
                    houses_serviced = provide_culture(x, y, religion_coverage_mercury);
                    break;
                case BUILDING_SMALL_TEMPLE_MARS:
                case BUILDING_LARGE_TEMPLE_MARS:
                    if (building_monument_gt_module_is_active(MARS_MODULE_1_MESS_HALL) && city_buildings_get_mess_hall()) {
                        collect_offerings(f->building_id, x, y);
                    }
                    houses_serviced = provide_culture(x, y, religion_coverage_mars);
                    break;
                case BUILDING_GRAND_TEMPLE_MARS:
                    houses_serviced = provide_culture(x, y, religion_coverage_mars);
                    break;
                case BUILDING_SMALL_TEMPLE_VENUS:
                case BUILDING_LARGE_TEMPLE_VENUS:
                    houses_serviced = provide_culture(x, y, religion_coverage_venus);
                    provide_market_goods(f->building_id, x, y);
                    if (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
                        provide_venus_wine_to_taverns(f->building_id, x, y);
                    }
                    break;
                case BUILDING_GRAND_TEMPLE_VENUS:
                    houses_serviced = provide_culture(x, y, religion_coverage_venus);
                    break;
                case BUILDING_PANTHEON:
                    houses_serviced = provide_culture(x, y, religion_coverage_ceres);
                    provide_culture(x, y, religion_coverage_neptune);
                    provide_culture(x, y, religion_coverage_mercury);
                    provide_culture(x, y, religion_coverage_mars);
                    provide_culture(x, y, religion_coverage_venus);
                    provide_culture(x, y, religion_coverage_pantheon);
                    break;
                default:
                    break;
            }
            break;
        case FIGURE_ACTOR:
            b = get_entertainment_building(f);
            if (b->type == BUILDING_THEATER) {
                houses_serviced = provide_culture(x, y, theater_coverage);
            } else if (b->type == BUILDING_AMPHITHEATER) {
                houses_serviced = provide_entertainment(x, y,
                    b->data.entertainment.days1 ? 2 : 1, amphitheater_coverage);
            }
            break;
        case FIGURE_GLADIATOR:
            b = get_entertainment_building(f);
            if (b->type == BUILDING_AMPHITHEATER) {
                houses_serviced = provide_entertainment(x, y,
                    b->data.entertainment.days2 ? 2 : 1, amphitheater_coverage);
            } else if (b->type == BUILDING_COLOSSEUM) {
                houses_serviced = provide_entertainment(x, y,
                    b->data.entertainment.days1 ? 2 : 1, colosseum_coverage);
            } else if (b->type == BUILDING_ARENA) {
                houses_serviced = provide_entertainment(x, y,
                    b->data.entertainment.days1 ? 2 : 1, arena_coverage);
            }
            break;
        case FIGURE_LION_TAMER:
            b = get_entertainment_building(f);
            if (b->type == BUILDING_ARENA) {
                houses_serviced = provide_entertainment(x, y,
                    b->data.entertainment.days1 ? 2 : 1, arena_coverage);
            } else {
                houses_serviced = provide_entertainment(x, y,
                    b->data.entertainment.days2 ? 2 : 1, colosseum_coverage);
            }
            break;
        case FIGURE_CHARIOTEER:
            houses_serviced = provide_culture(x, y, hippodrome_coverage);
            break;
        case FIGURE_BARKEEP:
            b = building_get(f->building_id);
            houses_serviced = provide_entertainment(x, y, b->data.market.inventory[INVENTORY_WINE] ? b->data.market.inventory[INVENTORY_MEAT] ? 2 : 1 : 0, tavern_coverage);
            break;
        case FIGURE_ENGINEER:
        case FIGURE_WORK_CAMP_ARCHITECT:
            {
                int max_damage = 0;
                houses_serviced = provide_service(x, y, &max_damage, engineer_coverage);
                if (max_damage > f->min_max_seen) {
                    f->min_max_seen = max_damage;
                } else if (f->min_max_seen <= 10) {
                    f->min_max_seen = 0;
                } else {
                    f->min_max_seen -= 10;
                }
                break;
            }
        case FIGURE_PREFECT:
            {
                int min_happiness = 100;
                houses_serviced = provide_service(x, y, &min_happiness, prefect_coverage);
                f->min_max_seen = min_happiness;
                break;
            }
        case FIGURE_RIOTER:
            if (f->terrain_usage == TERRAIN_USAGE_ENEMY) {
                if (figure_rioter_collapse_building(f) == 1) {
                    return 1;
                }
            }
            break;
        case FIGURE_TOURIST:
            tourist_visit(x, y, f, tourist_spend);
            break;
    }
    if (f->building_id) {
        b = building_get(f->building_id);
        b->houses_covered += houses_serviced;
        if (b->houses_covered > 300) {
            b->houses_covered = 300;
        }
    }
    return 0;
}
