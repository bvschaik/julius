#include "city_overlay_health.h"

#include "city/health.h"
#include "game/state.h"
#include "translation/translation.h"

static int show_building_barber(const building *b)
{
    return b->type == BUILDING_BARBER;
}

static int show_building_bathhouse(const building *b)
{
    return b->type == BUILDING_BATHHOUSE;
}

static int show_building_clinic(const building *b)
{
    return b->type == BUILDING_DOCTOR;
}

static int show_building_hospital(const building *b)
{
    return b->type == BUILDING_HOSPITAL;
}

static int show_building_sickness(const building *b)
{
    return b->type == BUILDING_HOSPITAL || b->type == BUILDING_DOCTOR ||
           b->type == BUILDING_BARBER || b->type == BUILDING_BATHHOUSE ||
           b->type == BUILDING_SMALL_MAUSOLEUM || b->type == BUILDING_LARGE_MAUSOLEUM;
}

static int show_figure_barber(const figure *f)
{
    return f->type == FIGURE_BARBER;
}

static int show_figure_bathhouse(const figure *f)
{
    return f->type == FIGURE_BATHHOUSE_WORKER;
}

static int show_figure_clinic(const figure *f)
{
    return f->type == FIGURE_DOCTOR;
}

static int show_figure_hospital(const figure *f)
{
    return f->type == FIGURE_SURGEON;
}

static int show_figure_sickness(const figure *f)
{
    return f->type == FIGURE_SURGEON || f->type == FIGURE_DOCTOR || f->type == FIGURE_DOCKER ||
           f->type == FIGURE_CART_PUSHER || f->type == FIGURE_TRADE_SHIP || f->type == FIGURE_WAREHOUSEMAN ||
           f->type == FIGURE_TRADE_CARAVAN || f->type == FIGURE_TRADE_CARAVAN_DONKEY || f->type == FIGURE_BARBER ||
           f->type == FIGURE_BATHHOUSE_WORKER;
}

static int get_column_height_barber(const building *b)
{
    return b->house_size && b->data.house.barber ? b->data.house.barber / 10 : NO_COLUMN;
}

static int get_column_height_bathhouse(const building *b)
{
    return b->house_size && b->data.house.bathhouse ? b->data.house.bathhouse / 10 : NO_COLUMN;
}

static int get_column_height_clinic(const building *b)
{
    return b->house_size && b->data.house.clinic ? b->data.house.clinic / 10 : NO_COLUMN;
}

static int get_column_height_hospital(const building *b)
{
    return b->house_size && b->data.house.hospital ? b->data.house.hospital / 10 : NO_COLUMN;
}

static int get_column_height_sickness(const building *b)
{
    return b->sickness_level ? b->sickness_level / 10 : NO_COLUMN;
}

static int get_tooltip_barber(tooltip_context *c, const building *b)
{
    if (b->data.house.barber <= 0) {
        return 31;
    } else if (b->data.house.barber >= 80) {
        return 32;
    } else if (b->data.house.barber < 20) {
        return 33;
    } else {
        return 34;
    }
}

static int get_tooltip_bathhouse(tooltip_context *c, const building *b)
{
    if (b->data.house.bathhouse <= 0) {
        return 8;
    } else if (b->data.house.bathhouse >= 80) {
        return 9;
    } else if (b->data.house.bathhouse >= 20) {
        return 10;
    } else {
        return 11;
    }
}

static int get_tooltip_clinic(tooltip_context *c, const building *b)
{
    if (b->data.house.clinic <= 0) {
        return 35;
    } else if (b->data.house.clinic >= 80) {
        return 36;
    } else if (b->data.house.clinic >= 20) {
        return 37;
    } else {
        return 38;
    }
}

static int get_tooltip_hospital(tooltip_context *c, const building *b)
{
    if (b->data.house.hospital <= 0) {
        return 39;
    } else if (b->data.house.hospital >= 80) {
        return 40;
    } else if (b->data.house.hospital >= 20) {
        return 41;
    } else {
        return 42;
    }
}

static int get_tooltip_sickness(tooltip_context *c, const building *b)
{
    if (building_is_house(b->type) || b->type == BUILDING_DOCK || b->type == BUILDING_WAREHOUSE || b->type == BUILDING_GRANARY) {
        if (b->sickness_level < 1) {
            c->translation_key = TR_TOOLTIP_OVERLAY_SICKNESS_NONE;
        } else if (b->sickness_level < LOW_SICKNESS_LEVEL) {
            c->translation_key = TR_TOOLTIP_OVERLAY_SICKNESS_LOW;
        } else if (b->sickness_level < MEDIUM_SICKNESS_LEVEL) {
            c->translation_key = TR_TOOLTIP_OVERLAY_SICKNESS_MEDIUM;
        } else if (b->sickness_level < HIGH_SICKNESS_LEVEL) {
            c->translation_key = TR_TOOLTIP_OVERLAY_SICKNESS_HIGH;
        } else {
            c->translation_key = TR_TOOLTIP_OVERLAY_SICKNESS_PLAGUE;
        }        
    }
    return 0;
}

const city_overlay *city_overlay_for_barber(void)
{
    static city_overlay overlay = {
        OVERLAY_BARBER,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_barber,
        show_figure_barber,
        get_column_height_barber,
        0,
        get_tooltip_barber,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_bathhouse(void)
{
    static city_overlay overlay = {
        OVERLAY_BATHHOUSE,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_bathhouse,
        show_figure_bathhouse,
        get_column_height_bathhouse,
        0,
        get_tooltip_bathhouse,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_clinic(void)
{
    static city_overlay overlay = {
        OVERLAY_CLINIC,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_clinic,
        show_figure_clinic,
        get_column_height_clinic,
        0,
        get_tooltip_clinic,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_hospital(void)
{
    static city_overlay overlay = {
        OVERLAY_HOSPITAL,
        COLUMN_COLOR_GREEN_TO_RED,
        show_building_hospital,
        show_figure_hospital,
        get_column_height_hospital,
        0,
        get_tooltip_hospital,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_sickness(void)
{
    static city_overlay overlay = {
            OVERLAY_SICKNESS,
            COLUMN_COLOR_RED_TO_GREEN,
            show_building_sickness,
            show_figure_sickness,
            get_column_height_sickness,
            0,
            get_tooltip_sickness,
            0,
            0
    };
    return &overlay;
}
