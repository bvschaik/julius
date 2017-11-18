#include "cityinfo.h"

#include "city/message.h"
#include "ui/allwindows.h"
#include "ui/videointermezzo.h"
#include "ui/window.h"

#include <sound>
#include <data>

#include "game/settings.h"
#include "game/time.h"
#include "graphics/mouse.h"

void CityInfo_Victory_check()
{
    if (Data_Scenario.isOpenPlay)
    {
        return;
    }
    int numCriteria = 0;
    Data_State.winState = WinState_Win;
    if (Data_Scenario.winCriteria.cultureEnabled)
    {
        numCriteria++;
        if (Data_CityInfo.ratingCulture < Data_Scenario.winCriteria.culture)
        {
            Data_State.winState = WinState_None;
        }
    }
    if (Data_Scenario.winCriteria.prosperityEnabled)
    {
        numCriteria++;
        if (Data_CityInfo.ratingProsperity < Data_Scenario.winCriteria.prosperity)
        {
            Data_State.winState = WinState_None;
        }
    }
    if (Data_Scenario.winCriteria.peaceEnabled)
    {
        numCriteria++;
        if (Data_CityInfo.ratingPeace < Data_Scenario.winCriteria.peace)
        {
            Data_State.winState = WinState_None;
        }
    }
    if (Data_Scenario.winCriteria.favorEnabled)
    {
        numCriteria++;
        if (Data_CityInfo.ratingFavor < Data_Scenario.winCriteria.favor)
        {
            Data_State.winState = WinState_None;
        }
    }
    if (Data_Scenario.winCriteria_populationEnabled)
    {
        numCriteria++;
        if (Data_CityInfo.population < Data_Scenario.winCriteria_population)
        {
            Data_State.winState = WinState_None;
        }
    }

    if (numCriteria <= 0)
    {
        Data_State.winState = WinState_None;
        if (Data_Scenario.winCriteria.timeLimitYearsEnabled)
        {
            numCriteria++;
        }
        if (Data_Scenario.winCriteria.survivalYearsEnabled)
        {
            numCriteria++;
        }
    }
    if (game_time_year() >= Data_Event.timeLimitMaxGameYear)
    {
        if (Data_Scenario.winCriteria.timeLimitYearsEnabled)
        {
            Data_State.winState = WinState_Lose;
        }
        else if (Data_Scenario.winCriteria.survivalYearsEnabled)
        {
            Data_State.winState = WinState_Win;
        }
    }
    if (Data_CityInfo.numImperialSoldiersInCity + Data_CityInfo.numEnemiesInCity > 2 + Data_CityInfo.numSoldiersInCity)
    {
        if (Data_CityInfo.population < Data_CityInfo.populationHighestEver / 4)
        {
            Data_State.winState = WinState_Lose;
        }
    }
    if (Data_CityInfo.numImperialSoldiersInCity + Data_CityInfo.numEnemiesInCity > 0)
    {
        if (Data_CityInfo.population <= 0)
        {
            Data_State.winState = WinState_Lose;
        }
    }
    if (numCriteria <= 0)
    {
        Data_State.winState = WinState_None;
    }
    if (Data_CityInfo.victoryHasWonScenario)
    {
        Data_State.winState = Data_CityInfo.victoryContinueMonths <= 0 ? WinState_Win : WinState_None;
    }
    if (Data_State.forceWinCheat)
    {
        Data_State.winState = WinState_Win;
    }
    if (Data_State.winState != WinState_None)
    {
        Data_State.selectedBuilding.type = 0;
        if (Data_State.winState == WinState_Lose)
        {
            if (Data_CityInfo.messageShownFired)
            {
                UI_Intermezzo_show(Intermezzo_Fired, Window_MissionEnd, 1000);
            }
            else
            {
                Data_CityInfo.messageShownFired = 1;
                city_message_post(1, MESSAGE_FIRED, 0, 0);
            }
            Data_State.forceWinCheat = 0;
        }
        else if (Data_State.winState == WinState_Win)
        {
            sound_music_stop();
            if (Data_CityInfo.messageShownVictory)
            {
                mouse_reset_up_state();

                if (IsTutorial1() || IsTutorial2())
                {
                    // tutorials: immediately go to next mission
                    UI_Window_goTo(Window_VictoryIntermezzo);
                }
                else if (!Data_Settings.isCustomScenario && Data_Settings.currentMissionId >= 10)
                {
                    // Won game
                    UI_VideoIntermezzo_show("smk/win_game.smk", 400, 292, Window_VictoryIntermezzo);
                }
                else
                {
                    if (setting_victory_video())
                    {
                        UI_VideoIntermezzo_show("smk/victory_balcony.smk", 400, 292, Window_VictoryIntermezzo);
                    }
                    else
                    {
                        UI_VideoIntermezzo_show("smk/victory_senate.smk", 400, 292, Window_VictoryIntermezzo);
                    }
                }
                Data_State.forceWinCheat = 0;
            }
            else
            {
                Data_CityInfo.messageShownVictory = 1;
                UI_Window_goTo(Window_VictoryDialog);
            }
        }
    }
}

void CityInfo_Victory_updateMonthsToGovern()
{
    if (Data_CityInfo.victoryHasWonScenario)
    {
        Data_CityInfo.victoryContinueMonths--;
    }
}
