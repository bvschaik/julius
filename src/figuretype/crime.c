#include "crime.h"

#include "building/building.h"
#include "building/destruction.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/sentiment.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/formation_enemy.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/tutorial.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

static const int CRIMINAL_OFFSETS[] = {
    0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

static void generate_rioter(building *b)
{
    int x_road, y_road;
    if (!map_closest_road_within_radius(b->x, b->y, b->size, 4, &x_road, &y_road)) {
        return;
    }
    Data_CityInfo.numCriminalsThisMonth++;
    int people_in_mob;
    if (Data_CityInfo.population <= 150) {
        people_in_mob = 1;
    } else if (Data_CityInfo.population <= 300) {
        people_in_mob = 2;
    } else if (Data_CityInfo.population <= 800) {
        people_in_mob = 3;
    } else if (Data_CityInfo.population <= 1200) {
        people_in_mob = 4;
    } else if (Data_CityInfo.population <= 2000) {
        people_in_mob = 5;
    } else {
        people_in_mob = 6;
    }
    int x_target, y_target;
    int target_building_id = formation_rioter_get_target_building(&x_target, &y_target);
    for (int i = 0; i < people_in_mob; i++) {
        figure *f = figure_create(FIGURE_RIOTER, x_road, y_road, DIR_4_BOTTOM);
        f->actionState = FIGURE_ACTION_120_RIOTER_CREATED;
        f->roamLength = 0;
        f->waitTicks = 10 + 4 * i;
        if (target_building_id) {
            f->destinationX = x_target;
            f->destinationY = y_target;
            f->destinationBuildingId = target_building_id;
        } else {
            f->state = FigureState_Dead;
        }
    }
    building_destroy_by_rioter(b);
    Data_CityInfo.ratingPeaceNumRiotersThisYear++;
    Data_CityInfo.riotCause = Data_CityInfo.populationEmigrationCause;
    city_sentiment_change_happiness(20);
    tutorial_on_crime();
    city_message_apply_sound_interval(MESSAGE_CAT_RIOT);
    city_message_post_with_popup_delay(MESSAGE_CAT_RIOT, MESSAGE_RIOT, b->type, map_grid_offset(x_road, y_road));
}

static void generate_mugger(building *b)
{
    Data_CityInfo.numCriminalsThisMonth++;
    if (b->houseCriminalActive < 2) {
        b->houseCriminalActive = 2;
        int xRoad, yRoad;
        if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
            figure *f = figure_create(FIGURE_CRIMINAL, xRoad, yRoad, DIR_4_BOTTOM);
            f->waitTicks = 10 + (b->houseGenerationDelay & 0xf);
            Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
            if (Data_CityInfo.financeTaxesThisYear > 20) {
                int moneyStolen = Data_CityInfo.financeTaxesThisYear / 4;
                if (moneyStolen > 400) {
                    moneyStolen = 400 - random_byte() / 2;
                }
                city_message_post(1, MESSAGE_THEFT, moneyStolen, f->gridOffset);
                city_finance_process_stolen(moneyStolen);
            }
        }
    }
}

static void generate_protestor(building *b)
{
    Data_CityInfo.numProtestersThisMonth++;
    if (b->houseCriminalActive < 1) {
        b->houseCriminalActive = 1;
        int xRoad, yRoad;
        if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
            figure *f = figure_create(FIGURE_PROTESTER, xRoad, yRoad, DIR_4_BOTTOM);
            f->waitTicks = 10 + (b->houseGenerationDelay & 0xf);
            Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
        }
    }
}

void figure_generate_criminals()
{
    building *min_building = 0;
    int min_happiness = 50;
    int max_id = building_get_highest_id();
    for (int i = 1; i <= max_id; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize) {
            if (b->sentiment.houseHappiness >= 50) {
                b->houseCriminalActive = 0;
            } else if (b->sentiment.houseHappiness < min_happiness) {
                min_happiness = b->sentiment.houseHappiness;
                min_building = b;
            }
        }
    }
    if (min_building) {
        if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
            return;
        }
        if (Data_CityInfo.citySentiment < 30) {
            if (random_byte() >= Data_CityInfo.citySentiment + 50) {
                if (min_happiness <= 10) {
                    generate_rioter(min_building);
                } else if (min_happiness < 30) {
                    generate_mugger(min_building);
                } else if (min_happiness < 50) {
                    generate_protestor(min_building);
                }
            }
        } else if (Data_CityInfo.citySentiment < 60) {
            if (random_byte() >= Data_CityInfo.citySentiment + 40) {
                if (min_happiness < 30) {
                    generate_mugger(min_building);
                } else if (min_happiness < 50) {
                    generate_protestor(min_building);
                }
            }
        } else {
            if (random_byte() >= Data_CityInfo.citySentiment + 20) {
                if (min_happiness < 50) {
                    generate_protestor(min_building);
                }
            }
        }
    }
}

void figure_protestor_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Roads;
    figure_image_increase_offset(f, 64);
    f->cartGraphicId = 0;
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->state = FigureState_Dead;
    }
    f->waitTicks++;
    if (f->waitTicks > 200) {
        f->state = FigureState_Dead;
        f->graphicOffset = 0;
    }
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->graphicOffset / 4] + 104;
    }
}

void figure_criminal_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Roads;
    figure_image_increase_offset(f, 32);
    f->cartGraphicId = 0;
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->state = FigureState_Dead;
    }
    f->waitTicks++;
    if (f->waitTicks > 200) {
        f->state = FigureState_Dead;
        f->graphicOffset = 0;
    }
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->graphicOffset / 2] + 104;
    }
}

void figure_rioter_action(figure *f)
{
    Data_CityInfo.numRiotersInCity++;
    if (!f->targetedByFigureId) {
        Data_CityInfo.riotersOrAttackingNativesInCity = 10;
    }
    f->terrainUsage = FigureTerrainUsage_Enemy;
    f->maxRoamLength = 480;
    f->cartGraphicId = 0;
    f->isGhost = 0;
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_120_RIOTER_CREATED:
            figure_image_increase_offset(f, 32);
            f->waitTicks++;
            if (f->waitTicks >= 160) {
                f->actionState = FIGURE_ACTION_121_RIOTER_MOVING;
                int x_tile, y_tile;
                int building_id = formation_rioter_get_target_building(&x_tile, &y_tile);
                if (building_id) {
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                    f->destinationBuildingId = building_id;
                    figure_route_remove(f);
                } else {
                    f->state = FigureState_Dead;
                }
            }
            break;
        case FIGURE_ACTION_121_RIOTER_MOVING:
            figure_image_increase_offset(f, 12);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                int x_tile, y_tile;
                int building_id = formation_rioter_get_target_building(&x_tile, &y_tile);
                if (building_id) {
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                    f->destinationBuildingId = building_id;
                    figure_route_remove(f);
                } else {
                    f->state = FigureState_Dead;
                }
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_120_RIOTER_CREATED;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_ATTACK) {
                if (f->graphicOffset > 12) {
                    f->graphicOffset = 0;
                }
            }
            break;
    }
    int dir;
    if (f->direction == DIR_FIGURE_ATTACK) {
        dir = f->attackDirection;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    dir = figure_image_normalize_direction(dir);
    
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + 96 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->graphicOffset];
    } else if (f->actionState == FIGURE_ACTION_121_RIOTER_MOVING) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + dir + 8 * f->graphicOffset;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->graphicOffset / 2];
    }
}

int figure_rioter_collapse_building(figure *f)
{
    for (int dir = 0; dir < 8; dir += 2) {
        int gridOffset = f->gridOffset + map_grid_direction_delta(dir);
        if (!map_building_at(gridOffset)) {
            continue;
        }
        building *b = building_get(map_building_at(gridOffset));
        switch (b->type) {
            case BUILDING_WAREHOUSE_SPACE:
            case BUILDING_WAREHOUSE:
            case BUILDING_FORT_GROUND:
            case BUILDING_FORT:
            case BUILDING_BURNING_RUIN:
                continue;
        }
        if (b->houseSize && b->subtype.houseLevel < HOUSE_SMALL_CASA) {
            continue;
        }
        city_message_apply_sound_interval(MESSAGE_CAT_RIOT_COLLAPSE);
        city_message_post(0, MESSAGE_DESTROYED_BUILDING, b->type, f->gridOffset);
        city_message_increase_category_count(MESSAGE_CAT_RIOT_COLLAPSE);
        building_destroy_by_rioter(b);
        f->actionState = FIGURE_ACTION_120_RIOTER_CREATED;
        f->waitTicks = 0;
        f->direction = dir;
        return 1;
    }
    return 0;
}
