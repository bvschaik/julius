#include "city_overlay_risks.h"

#include "building/industry.h"
#include "figure/properties.h"
#include "game/state.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "translation/translation.h"
#include "widget/city_draw_highway.h"

enum crime_level {
    NO_CRIME = 0,
    MINOR_CRIME = 1,
    LOW_CRIME = 2,
    SOME_CRIME = 3,
    MEDIUM_CRIME = 4,
    LARGE_CRIME = 5,
    RAMPANT_CRIME = 6,
};

static int is_problem_cartpusher(int figure_id)
{
    if (figure_id) {
        figure *fig = figure_get(figure_id);
        return fig->action_state == FIGURE_ACTION_20_CARTPUSHER_INITIAL && fig->min_max_seen;
    } else {
        return 0;
    }
}

void city_overlay_problems_prepare_building(building *b)
{
    b = building_main(b);

    if (b->strike_duration_days > 0) {
        b->show_on_problem_overlay = 1;
        return;
    }

    if (b->has_plague) {
        b->show_on_problem_overlay = 1;
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        b->show_on_problem_overlay = 1;
    } else if (!b->num_workers && building_get_laborers(b->type)) {
        b->show_on_problem_overlay = 1;
    } else if (b->type == BUILDING_FOUNTAIN || b->type == BUILDING_BATHHOUSE) {
        if (!b->has_water_access) {
            b->show_on_problem_overlay = 1;
        }
    } else if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_CLAY_PIT) {
        if (is_problem_cartpusher(b->figure_id)) {
            b->show_on_problem_overlay = 1;
        }
    } else if (building_is_workshop(b->type)) {
        if (is_problem_cartpusher(b->figure_id)) {
            b->show_on_problem_overlay = 1;
        } else if (b->loads_stored <= 0) {
            b->show_on_problem_overlay = 1;
        }
    } else if ((b->type == BUILDING_THEATER || b->type == BUILDING_AMPHITHEATER || b->type == BUILDING_ARENA ||
        b->type == BUILDING_COLOSSEUM || b->type == BUILDING_HIPPODROME) && !b->data.entertainment.days1) {
        b->show_on_problem_overlay = 1;
    } else if ((b->type == BUILDING_ARENA || b->type == BUILDING_COLOSSEUM) && !b->data.entertainment.days2) {
        b->show_on_problem_overlay = 1;
    }

    if (b->show_on_problem_overlay) {
        while (b->next_part_building_id) {
            b = building_get(b->next_part_building_id);
            b->show_on_problem_overlay = 1;
        }
    }
}

static int show_building_fire_crime(const building *b)
{
    return b->type == BUILDING_PREFECTURE || b->type == BUILDING_BURNING_RUIN;
}

static int show_building_damage(const building *b)
{
    return b->type == BUILDING_ENGINEERS_POST || b->type == BUILDING_ARCHITECT_GUILD;
}

static int show_building_problems(const building *b)
{
    return b->show_on_problem_overlay;
}

static int show_building_native(const building *b)
{
    return b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING || b->type == BUILDING_MISSION_POST;
}

static int show_building_none(const building *b)
{
    return 0;
}

static int show_figure_fire(const figure *f)
{
    return f->type == FIGURE_PREFECT;
}

static int show_figure_damage(const figure *f)
{
    return f->type == FIGURE_ENGINEER || f->type == FIGURE_WORK_CAMP_ARCHITECT;
}

static int show_figure_crime(const figure *f)
{
    return f->type == FIGURE_PREFECT ||
        f->type == FIGURE_CRIMINAL || f->type == FIGURE_RIOTER || f->type == FIGURE_PROTESTER
        || f->type == FIGURE_CRIMINAL_LOOTER || f->type == FIGURE_CRIMINAL_ROBBER;
}

static int show_figure_problems(const figure *f)
{
    if (f->type == FIGURE_LABOR_SEEKER) {
        return building_get(f->building_id)->show_on_problem_overlay;
    } else if (f->type == FIGURE_CART_PUSHER) {
        return f->action_state == FIGURE_ACTION_20_CARTPUSHER_INITIAL || f->min_max_seen;
    } else if (f->type == FIGURE_PROTESTER) {
        return 1;
    } else {
        return 0;
    }
}

static int show_figure_native(const figure *f)
{
    return f->type == FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_MISSIONARY;
}

static int show_figure_enemy(const figure *f)
{
    const figure_properties *props = figure_properties_for_type(f->type);
    return props->category == FIGURE_CATEGORY_HOSTILE || props->category == FIGURE_CATEGORY_NATIVE;
}

static int get_column_height_fire(const building *b)
{
    return b->fire_risk > 0 ? b->fire_risk / 10 : NO_COLUMN;
}

static int get_column_height_damage(const building *b)
{
    return b->damage_risk > 0 ? b->damage_risk / 20 : NO_COLUMN;
}

static int get_crime_level(const building *b)
{
    if (b->house_size) {
        int happiness = b->sentiment.house_happiness;
        if (happiness <= 0) {
            return RAMPANT_CRIME;
        } else if (happiness <= 10 || b->house_criminal_active) {
            return LARGE_CRIME;
        } else if (happiness <= 20) {
            return MEDIUM_CRIME;
        } else if (happiness <= 30) {
            return SOME_CRIME;
        } else if (happiness <= 40) {
            return LOW_CRIME;
        } else if (happiness < 50) {
            return MINOR_CRIME;
        }
    }
    return 0;

}

static int get_column_height_crime(const building *b)
{
    if (b->house_size) {
        int crime = get_crime_level(b);
        if (crime == RAMPANT_CRIME) {
            return 10;
        } else if (crime == LARGE_CRIME) {
            return 8;
        } else if (crime == MEDIUM_CRIME) {
            return 6;
        } else if (crime == SOME_CRIME) {
            return 4;
        } else if (crime == LOW_CRIME) {
            return 2;
        } else if (crime == MINOR_CRIME) {
            return 1;
        }
    }
    return NO_COLUMN;
}

static int get_column_height_none(const building *b)
{
    return NO_COLUMN;
}

static int get_tooltip_fire(tooltip_context *c, const building *b)
{
    if (b->fire_risk <= 0) {
        return 46;
    } else if (b->fire_risk <= 20) {
        return 47;
    } else if (b->fire_risk <= 40) {
        return 48;
    } else if (b->fire_risk <= 60) {
        return 49;
    } else if (b->fire_risk <= 80) {
        return 50;
    } else {
        return 51;
    }
}

static int get_tooltip_damage(tooltip_context *c, const building *b)
{
    if (b->damage_risk <= 0) {
        return 52;
    } else if (b->damage_risk <= 40) {
        return 53;
    } else if (b->damage_risk <= 80) {
        return 54;
    } else if (b->damage_risk <= 120) {
        return 55;
    } else if (b->damage_risk <= 160) {
        return 56;
    } else {
        return 57;
    }
}

static int get_tooltip_crime(tooltip_context *c, const building *b)
{
    int crime = get_crime_level(b);
    if (crime == RAMPANT_CRIME) {
        return 63;
    } else if (crime == LARGE_CRIME) {
        return 62;
    } else if (crime == MEDIUM_CRIME) {
        return 61;
    } else if (crime == SOME_CRIME) {
        return 60;
    } else if (crime == LOW_CRIME || crime == MINOR_CRIME) {
        return 59;
    } else {
        return 58;
    }
}

static int get_tooltip_problems(tooltip_context *c, const building *b)
{
    const building *main_building = b;

    int guard = 0;
    while (guard < 9) {
        if (main_building->prev_part_building_id <= 0) {
            break;
        }
        main_building = building_get(main_building->prev_part_building_id);
        guard++;
    }
    if (guard < 9) {
        b = main_building;
    }
    if (b->has_plague) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_PLAGUE;
    }
    if (b->strike_duration_days > 0) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_STRIKE;
    } else if (b->state == BUILDING_STATE_MOTHBALLED) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_MOTHBALLED;
    } else if (!b->num_workers && building_get_laborers(b->type)) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_NO_LABOR;
    } else if (b->type == BUILDING_FOUNTAIN || b->type == BUILDING_BATHHOUSE) {
        c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_NO_WATER_ACCESS;
    } else if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_CLAY_PIT) {
        if (is_problem_cartpusher(b->figure_id)) {
            c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_CARTPUSHER;
        }
    } else if (building_is_workshop(b->type)) {
        if (is_problem_cartpusher(b->figure_id)) {
            c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_CARTPUSHER;
        } else if (b->loads_stored <= 0) {
            c->translation_key = TR_TOOLTIP_OVERLAY_PROBLEMS_NO_RESOURCES;
        }
    } else if (b->type == BUILDING_THEATER && !b->data.entertainment.days1) {
        c->text_group = 72;
        return 5;
    } else if (b->type == BUILDING_AMPHITHEATER) {
        if (!b->data.entertainment.days1) {
            c->text_group = 71;
            return 7;
        } else if (!b->data.entertainment.days2) {
            c->text_group = 71;
            return 9;
        }
    } else if (b->type == BUILDING_ARENA || b->type == BUILDING_COLOSSEUM) {
        if (!b->data.entertainment.days1) {
            c->text_group = 74;
            return 7;
        } else if (!b->data.entertainment.days2) {
            c->text_group = 74;
            return 9;
        }
    } else if (b->type == BUILDING_HIPPODROME && !b->data.entertainment.days1) {
        c->text_group = 73;
        return 5;
    }
    if (c->translation_key) {
        return 1;
    }
    return 0;
}

const city_overlay *city_overlay_for_fire(void)
{
    static city_overlay overlay = {
        OVERLAY_FIRE,
        COLUMN_COLOR_RED_TO_GREEN,
        show_building_fire_crime,
        show_figure_fire,
        get_column_height_fire,
        0,
        get_tooltip_fire,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_damage(void)
{
    static city_overlay overlay = {
        OVERLAY_DAMAGE,
        COLUMN_COLOR_RED_TO_GREEN,
        show_building_damage,
        show_figure_damage,
        get_column_height_damage,
        0,
        get_tooltip_damage,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_crime(void)
{
    static city_overlay overlay = {
        OVERLAY_CRIME,
        COLUMN_COLOR_RED_TO_GREEN,
        show_building_fire_crime,
        show_figure_crime,
        get_column_height_crime,
        0,
        get_tooltip_crime,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_problems(void)
{
    static city_overlay overlay = {
        OVERLAY_PROBLEMS,
        COLUMN_COLOR_RED,
        show_building_problems,
        show_figure_problems,
        get_column_height_none,
        0,
        get_tooltip_problems,
        0,
        0
    };
    return &overlay;
}

static int terrain_on_native_overlay(void)
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB |
        TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE;
}

static void draw_footprint_native(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    if (map_terrain_is(grid_offset, terrain_on_native_overlay())) {
        if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
        } else {
            image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0, scale);
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT | TERRAIN_WALL)) {
        // display grass
        int image_id = image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(grid_offset) & 7);
        image_draw_isometric_footprint_from_draw_tile(image_id, x, y, 0, scale);
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        city_with_overlay_draw_building_footprint(x, y, grid_offset, 0);
    } else {
        if (map_property_is_native_land(grid_offset)) {
            image_draw_isometric_footprint_from_draw_tile(image_group(GROUP_TERRAIN_DESIRABILITY) + 1, x, y, 0, scale);
        } else if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY)) {
            city_draw_highway_footprint(x, y, scale, grid_offset);
        } else {
            image_draw_isometric_footprint_from_draw_tile(map_image_at(grid_offset), x, y, 0, scale);
        }
    }
}

static void draw_top_native(int x, int y, float scale, int grid_offset)
{
    if (!map_property_is_draw_tile(grid_offset)) {
        return;
    }
    if (map_terrain_is(grid_offset, terrain_on_native_overlay())) {
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
            color_t color_mask = 0;
            if (map_property_is_deleted(grid_offset) && map_property_multi_tile_size(grid_offset) == 1) {
                color_mask = COLOR_MASK_RED;
            }
            image_draw_isometric_top_from_draw_tile(map_image_at(grid_offset), x, y, color_mask, scale);
        }
    } else if (map_building_at(grid_offset)) {
        city_with_overlay_draw_building_top(x, y, grid_offset);
    }
}

const city_overlay *city_overlay_for_native(void)
{
    static city_overlay overlay = {
        OVERLAY_NATIVE,
        COLUMN_COLOR_RED,
        show_building_native,
        show_figure_native,
        get_column_height_none,
        0,
        0,
        draw_footprint_native,
        draw_top_native
    };
    return &overlay;
}


const city_overlay *city_overlay_for_enemy(void)
{
    static city_overlay overlay = {
        OVERLAY_ENEMY,
        COLUMN_COLOR_RED,
        show_building_none,
        show_figure_enemy,
        get_column_height_none,
        0,
        0,
        0,
        0
    };
    return &overlay;
}
