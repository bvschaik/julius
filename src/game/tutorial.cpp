#include "tutorial.h"

#include "playermessage.h"
#include "sidebarmenu.h"

#include "data/cityinfo.hpp"
#include "data/message.hpp"
#include "data/scenario.hpp"
#include "data/settings.hpp"

#include "game/resource.h"
#include "game/time.h"

struct
{
    struct
    {
        int fire;
        int crime;
        int collapse;
        int senate_built;
    } tutorial1;
    struct
    {
        int granary_built;
        int population_250_reached;
        int population_450_reached;
        int pottery_made;
        int pottery_made_year;
    } tutorial2;
    struct
    {
        int disease;
    } tutorial3;
} data;

void Tutorial::init()
{
    int tut1 = 1, tut2 = 1, tut3 = 1;
    if (IsTutorial1())
    {
        tut1 = tut2 = 0;
    }
    else if (IsTutorial2())
    {
        tut2 = 0;
    }
    else if (IsTutorial3())
    {
        tut3 = 0;
    }

    data.tutorial1.fire = tut1;
    data.tutorial1.crime = tut1;
    data.tutorial1.collapse = tut1;
    data.tutorial1.senate_built = tut1;
    Data_CityInfo.tutorial1FireMessageShown = tut1;

    data.tutorial2.granary_built = tut2;
    data.tutorial2.population_250_reached = tut2;
    data.tutorial2.population_450_reached = tut2;
    data.tutorial2.pottery_made = tut2;
    data.tutorial2.pottery_made_year = tut2;

    data.tutorial3.disease = tut3;
    Data_CityInfo.tutorial3DiseaseMessageShown = tut3;
}

tutorial_availability Tutorial::advisor_empire_availability()
{
    if (IsTutorial1())
    {
        return NOT_AVAILABLE;
    }
    else if (IsTutorial2() && !data.tutorial2.population_250_reached)
    {
        return NOT_AVAILABLE_YET;
    }
    else
    {
        return AVAILABLE;
    }
}

tutorial_build_buttons Tutorial::get_build_buttons()
{
    if (IsTutorial1())
    {
        if (!data.tutorial1.fire && !data.tutorial1.crime)
        {
            return TUT1_BUILD_START;
        }
        else if (!data.tutorial1.collapse)
        {
            return TUT1_BUILD_AFTER_FIRE;
        }
        else if (!data.tutorial1.senate_built)
        {
            return TUT1_BUILD_AFTER_COLLAPSE;
        }
    }
    else if (IsTutorial2())
    {
        if (!data.tutorial2.granary_built)
        {
            return TUT2_BUILD_START;
        }
        else if (!data.tutorial2.population_250_reached)
        {
            return TUT2_BUILD_UP_TO_250;
        }
        else if (!data.tutorial2.population_450_reached)
        {
            return TUT2_BUILD_UP_TO_450;
        }
        else if (!data.tutorial2.pottery_made)
        {
            return TUT2_BUILD_AFTER_450;
        }
    }
    return TUT_BUILD_NORMAL;
}

int Tutorial::get_population_cap(int current_cap)
{
    if (IsTutorial1())
    {
        if (!data.tutorial1.fire ||
                !data.tutorial1.collapse ||
                !data.tutorial1.senate_built)
        {
            return 80;
        }
    }
    else if (IsTutorial2())
    {
        if (!data.tutorial2.granary_built)
        {
            return 150;
        }
        else if (!data.tutorial2.pottery_made)
        {
            return 520;
        }
    }
    return current_cap;
}

int Tutorial::get_immediate_goal_text()
{
    if (IsTutorial1())
    {
        if (!data.tutorial1.fire && !data.tutorial1.crime)
        {
            return 17;
        }
        else if (!data.tutorial1.collapse)
        {
            return 18;
        }
        else if (!data.tutorial1.senate_built)
        {
            return 19;
        }
        else
        {
            return 20;
        }
    }
    else if (IsTutorial2())
    {
        if (!data.tutorial2.granary_built)
        {
            return 21;
        }
        else if (!data.tutorial2.population_250_reached)
        {
            return 22;
        }
        else if (!data.tutorial2.population_450_reached)
        {
            return 23;
        }
        else if (!data.tutorial2.pottery_made)
        {
            return 24;
        }
        else
        {
            return 25;
        }
    }
    return 0;
}

int Tutorial::adjust_request_year(int *year)
{
    if (IsTutorial2())
    {
        if (!data.tutorial2.pottery_made)
        {
            return 0;
        }
        *year = data.tutorial2.pottery_made_year;
    }
    return 1;
}

int Tutorial::extra_fire_risk()
{
    return !data.tutorial1.fire;
}

int Tutorial::extra_damage_risk()
{
    return data.tutorial1.fire && !data.tutorial1.collapse;
}


static void refresh_buttons()
{
    SidebarMenu_enableBuildingMenuItemsAndButtons();
}

static void post_message(int message)
{
    PlayerMessage_post(1, message, 0, 0);
}

int Tutorial::handle_fire()
{
    if (data.tutorial1.fire)
    {
        return 0;
    }
    data.tutorial1.fire = 1;
    refresh_buttons();
    post_message(Message_53_TutorialFirstFire);
    return 1;
}

int Tutorial::handle_collapse()
{
    if (data.tutorial1.collapse)
    {
        return 0;
    }
    data.tutorial1.collapse = 1;
    refresh_buttons();
    post_message(Message_54_TutorialFirstCollapse);
    return 1;
}

void Tutorial::on_crime()
{
    if (!data.tutorial1.crime)
    {
        data.tutorial1.crime = 1;
        refresh_buttons();
    }
}

void Tutorial::on_disease()
{
    data.tutorial3.disease = 1;
}

void Tutorial::on_filled_granary()
{
    if (!data.tutorial2.granary_built)
    {
        data.tutorial2.granary_built = 1;
        refresh_buttons();
        post_message(Message_56_TutorialProvidingWater);
    }
}

void Tutorial::on_add_to_warehouse()
{
    if (Data_CityInfo.resourceStored[RESOURCE_POTTERY] >= 1 &&
            !data.tutorial2.pottery_made)
    {
        data.tutorial2.pottery_made = 1;
        data.tutorial2.pottery_made_year = game_time_year();
        refresh_buttons();
        post_message(Message_61_TutorialTrade);
    }
}

void Tutorial::on_day_tick()
{
    if (data.tutorial1.fire && !Data_CityInfo.tutorial1FireMessageShown)
    {
        Data_CityInfo.tutorial1FireMessageShown = 1;
    }
    if (data.tutorial3.disease && !Data_CityInfo.tutorial3DiseaseMessageShown)
    {
        Data_CityInfo.tutorial3DiseaseMessageShown = 1;
        post_message(Message_119_TutorialHealth);
    }
    if (data.tutorial2.granary_built)
    {
        if (!data.tutorial2.population_250_reached && Data_CityInfo.population >= 250)
        {
            data.tutorial2.population_250_reached = 1;
            refresh_buttons();
            post_message(Message_57_TutorialGrowingYourCity);
        }
    }
    if (data.tutorial2.population_250_reached)
    {
        if (!data.tutorial2.population_450_reached && Data_CityInfo.population >= 450)
        {
            data.tutorial2.population_450_reached = 1;
            refresh_buttons();
            post_message(Message_60_TutorialTaxesIndustry);
        }
    }
    if (data.tutorial1.fire && !data.tutorial1.senate_built)
    {
        int population_almost = Data_CityInfo.population >= Data_Scenario.winCriteria_population - 20;
        if (!game_time_day() || population_almost)
        {
            if (Data_CityInfo.buildingSenateGridOffset)
            {
                Data_CityInfo.tutorial1SenateBuilt++;
            }
            if (Data_CityInfo.tutorial1SenateBuilt > 0 || population_almost)
            {
                data.tutorial1.senate_built = 1;
                refresh_buttons();
                post_message(Message_59_TutorialReligion);
            }
        }
    }
}

void Tutorial::on_month_tick()
{
    if (IsTutorial3())
    {
        if (game_time_month() == 5)
        {
            if (Data_Message.messageDelay[MessageDelay_Tutorial3] <= 0)
            {
                Data_Message.messageDelay[MessageDelay_Tutorial3] = 1200;
                post_message(Message_58_TutorialHungerHaltsImmigrants);
            }
        }
    }
}

void Tutorial::save_state(buffer *buf1, buffer *buf2, buffer *buf3)
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


void Tutorial::load_state(buffer *buf1, buffer *buf2, buffer *buf3)
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
