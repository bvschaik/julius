#include "city_overlay_education.h"

#include "game/state.h"

static int show_building_education(const building *b)
{
    return b->type == BUILDING_SCHOOL || b->type == BUILDING_LIBRARY || b->type == BUILDING_ACADEMY;
}

static int show_building_school(const building *b)
{
    return b->type == BUILDING_SCHOOL;
}

static int show_building_library(const building *b)
{
    return b->type == BUILDING_LIBRARY;
}

static int show_building_academy(const building *b)
{
    return b->type == BUILDING_ACADEMY;
}

static int show_figure_education(const figure *f)
{
    return f->type == FIGURE_SCHOOL_CHILD || f->type == FIGURE_LIBRARIAN || f->type == FIGURE_TEACHER;
}

static int show_figure_school(const figure *f)
{
    return f->type == FIGURE_SCHOOL_CHILD;
}

static int show_figure_library(const figure *f)
{
    return f->type == FIGURE_LIBRARIAN;
}

static int show_figure_academy(const figure *f)
{
    return f->type == FIGURE_TEACHER;
}

static int get_column_height_education(const building *b)
{
    return b->house_size && b->data.house.education ? b->data.house.education * 3 - 1 : NO_COLUMN;
}

static int get_column_height_school(const building *b)
{
    return b->house_size && b->data.house.school ? b->data.house.school / 10 : NO_COLUMN;
}

static int get_column_height_library(const building *b)
{
    return b->house_size && b->data.house.library ? b->data.house.library / 10 : NO_COLUMN;
}

static int get_column_height_academy(const building *b)
{
    return b->house_size && b->data.house.academy ? b->data.house.academy / 10 : NO_COLUMN;
}

static int get_tooltip_education(tooltip_context *c, const building *b)
{
    switch (b->data.house.education) {
        case 0: return 100;
        case 1: return 101;
        case 2: return 102;
        case 3: return 103;
        default: return 0;
    }
}

static int get_tooltip_school(tooltip_context *c, const building *b)
{
    if (b->data.house.school <= 0) {
        return 19;
    } else if (b->data.house.school >= 80) {
        return 20;
    } else if (b->data.house.school >= 20) {
        return 21;
    } else {
        return 22;
    }
}

static int get_tooltip_library(tooltip_context *c, const building *b)
{
    if (b->data.house.library <= 0) {
        return 23;
    } else if (b->data.house.library >= 80) {
        return 24;
    } else if (b->data.house.library >= 20) {
        return 25;
    } else {
        return 26;
    }
}

static int get_tooltip_academy(tooltip_context *c, const building *b)
{
    if (b->data.house.academy <= 0) {
        return 27;
    } else if (b->data.house.academy >= 80) {
        return 28;
    } else if (b->data.house.academy >= 20) {
        return 29;
    } else {
        return 30;
    }
}

const city_overlay *city_overlay_for_education(void)
{
    static city_overlay overlay = {
        OVERLAY_EDUCATION,
        COLUMN_TYPE_ACCESS,
        show_building_education,
        show_figure_education,
        get_column_height_education,
        0,
        get_tooltip_education,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_school(void)
{
    static city_overlay overlay = {
        OVERLAY_SCHOOL,
        COLUMN_TYPE_ACCESS,
        show_building_school,
        show_figure_school,
        get_column_height_school,
        0,
        get_tooltip_school,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_library(void)
{
    static city_overlay overlay = {
        OVERLAY_LIBRARY,
        COLUMN_TYPE_ACCESS,
        show_building_library,
        show_figure_library,
        get_column_height_library,
        0,
        get_tooltip_library,
        0,
        0
    };
    return &overlay;
}

const city_overlay *city_overlay_for_academy(void)
{
    static city_overlay overlay = {
        OVERLAY_ACADEMY,
        COLUMN_TYPE_ACCESS,
        show_building_academy,
        show_figure_academy,
        get_column_height_academy,
        0,
        get_tooltip_academy,
        0,
        0
    };
    return &overlay;
}
