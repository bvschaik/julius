#include "CityInfo.h"

#include "UI/AllWindows.h"
#include "UI/VideoIntermezzo.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "city/message.h"
#include "game/settings.h"
#include "game/time.h"
#include "input/mouse.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "sound/music.h"

void CityInfo_Victory_check()
{
	if (scenario_is_open_play()) {
		return;
	}
	int numCriteria = 0;
	Data_State.winState = WinState_Win;
	if (scenario_criteria_culture_enabled()) {
		numCriteria++;
		if (Data_CityInfo.ratingCulture < scenario_criteria_culture()) {
			Data_State.winState = WinState_None;
		}
	}
	if (scenario_criteria_prosperity_enabled()) {
		numCriteria++;
		if (Data_CityInfo.ratingProsperity < scenario_criteria_prosperity()) {
			Data_State.winState = WinState_None;
		}
	}
	if (scenario_criteria_peace_enabled()) {
		numCriteria++;
		if (Data_CityInfo.ratingPeace < scenario_criteria_peace()) {
			Data_State.winState = WinState_None;
		}
	}
	if (scenario_criteria_favor_enabled()) {
		numCriteria++;
		if (Data_CityInfo.ratingFavor < scenario_criteria_favor()) {
			Data_State.winState = WinState_None;
		}
	}
	if (scenario_criteria_population_enabled()) {
		numCriteria++;
		if (Data_CityInfo.population < scenario_criteria_population()) {
			Data_State.winState = WinState_None;
		}
	}

	if (numCriteria <= 0) {
		Data_State.winState = WinState_None;
		if (scenario_criteria_time_limit_enabled()) {
			numCriteria++;
		}
		if (scenario_criteria_survival_enabled()) {
			numCriteria++;
		}
	}
	if (game_time_year() >= scenario_criteria_max_year()) {
		if (scenario_criteria_time_limit_enabled()) {
			Data_State.winState = WinState_Lose;
		} else if (scenario_criteria_survival_enabled()) {
			Data_State.winState = WinState_Win;
		}
	}
	if (Data_CityInfo.numImperialSoldiersInCity + Data_CityInfo.numEnemiesInCity > 2 + Data_CityInfo.numSoldiersInCity) {
		if (Data_CityInfo.population < Data_CityInfo.populationHighestEver / 4) {
			Data_State.winState = WinState_Lose;
		}
	}
	if (Data_CityInfo.numImperialSoldiersInCity + Data_CityInfo.numEnemiesInCity > 0) {
		if (Data_CityInfo.population <= 0) {
			Data_State.winState = WinState_Lose;
		}
	}
	if (numCriteria <= 0) {
		Data_State.winState = WinState_None;
	}
	if (Data_CityInfo.victoryHasWonScenario) {
		Data_State.winState = Data_CityInfo.victoryContinueMonths <= 0 ? WinState_Win : WinState_None;
	}
	if (Data_State.forceWinCheat) {
		Data_State.winState = WinState_Win;
	}
	if (Data_State.winState != WinState_None) {
		Data_State.selectedBuilding.type = 0;
		if (Data_State.winState == WinState_Lose) {
			if (Data_CityInfo.messageShownFired) {
				UI_Intermezzo_show(Intermezzo_Fired, Window_MissionEnd, 1000);
			} else {
				Data_CityInfo.messageShownFired = 1;
				game.messages.post(1, MESSAGE_FIRED, 0, 0);
			}
			Data_State.forceWinCheat = 0;
		} else if (Data_State.winState == WinState_Win) {
			sound_music_stop();
			if (Data_CityInfo.messageShownVictory) {
				mouse_reset_up_state();
				
				if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
					// tutorials: immediately go to next mission
					UI_Window_goTo(Window_VictoryIntermezzo);
				} else if (!scenario_is_custom() && scenario_campaign_rank() >= 10) {
					// Won game
					UI_VideoIntermezzo_show("smk/win_game.smk", 400, 292, Window_VictoryIntermezzo);
				} else {
					if (setting_victory_video()) {
						UI_VideoIntermezzo_show("smk/victory_balcony.smk", 400, 292, Window_VictoryIntermezzo);
					} else {
						UI_VideoIntermezzo_show("smk/victory_senate.smk", 400, 292, Window_VictoryIntermezzo);
					}
				}
				Data_State.forceWinCheat = 0;
			} else {
				Data_CityInfo.messageShownVictory = 1;
				UI_Window_goTo(Window_VictoryDialog);
			}
		}
	}
}

void CityInfo_Victory_updateMonthsToGovern()
{
	if (Data_CityInfo.victoryHasWonScenario) {
		Data_CityInfo.victoryContinueMonths--;
	}
}
