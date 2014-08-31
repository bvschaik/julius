#include "Settings.h"

#include "Data/Model.h"
#include "Data/Settings.h"

void Settings_clearMissionSettings()
{
	for (int i = 0; i < 100; i++) {
		Data_Settings.personalSavingsPerMission[i] = 0;
	}
	Data_Settings.startingFavor = Data_Model_Difficulty.startingFavor[Data_Settings.difficulty];
	Data_Settings.personalSavingsLastMission = 0;
	Data_Settings.currentMissionId = 0;
	Data_Settings.saveGameMissionId = 0;
}
