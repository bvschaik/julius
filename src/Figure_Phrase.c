#include "Building.h"

#include "Data/CityInfo.h"

#include "figure/figure.h"
#include "figure/trader.h"
#include "figure/type.h"
#include "sound/effect.h"
#include "sound/speech.h"

void Figure_playDieSound(int figureType)
{
	int isSoldier = 0;
	int isCitizen = 0;
	switch (figureType) {
		case FIGURE_WOLF:
			sound_effect_play(SOUND_EFFECT_WOLF_DIE);
			break;
		case FIGURE_SHEEP:
			sound_effect_play(SOUND_EFFECT_SHEEP_DIE);
			break;
		case FIGURE_ZEBRA:
			sound_effect_play(SOUND_EFFECT_ZEBRA_DIE);
			break;
		case FIGURE_LION_TAMER:
			sound_effect_play(SOUND_EFFECT_LION_DIE);
			break;
		case FIGURE_ENEMY48_CHARIOT:
		case FIGURE_ENEMY52_MOUNTED_ARCHER:
			sound_effect_play(SOUND_EFFECT_HORSE2);
			break;
		case FIGURE_ENEMY46_CAMEL:
			sound_effect_play(SOUND_EFFECT_CAMEL);
			break;
		case FIGURE_ENEMY47_ELEPHANT:
			sound_effect_play(SOUND_EFFECT_ELEPHANT_DIE);
			break;
		case FIGURE_NATIVE_TRADER:
		case FIGURE_TRADE_CARAVAN:
		case FIGURE_TRADE_CARAVAN_DONKEY:
			break;
		case FIGURE_PREFECT:
		case FIGURE_FORT_JAVELIN:
		case FIGURE_FORT_MOUNTED:
		case FIGURE_FORT_LEGIONARY:
		case FIGURE_GLADIATOR:
		case FIGURE_INDIGENOUS_NATIVE:
		case FIGURE_TOWER_SENTRY:
		case FIGURE_ENEMY43_SPEAR:
		case FIGURE_ENEMY44_SWORD:
		case FIGURE_ENEMY45_SWORD:
		case FIGURE_ENEMY49_FAST_SWORD:
		case FIGURE_ENEMY50_SWORD:
		case FIGURE_ENEMY51_SPEAR:
		case FIGURE_ENEMY53_AXE:
		case FIGURE_ENEMY54_GLADIATOR:
		case FIGURE_ENEMY55_JAVELIN:
		case FIGURE_ENEMY56_MOUNTED:
		case FIGURE_ENEMY_CAESAR_LEGIONARY:
			isSoldier = 1;
			break;
		default:
			isCitizen = 1;
			break;
	}
	if (isSoldier) {
		Data_CityInfo.dieSoundSoldier++;
		if (Data_CityInfo.dieSoundSoldier >= 4) {
			Data_CityInfo.dieSoundSoldier = 0;
		}
		sound_effect_play(SOUND_EFFECT_SOLDIER_DIE + Data_CityInfo.dieSoundSoldier);
	} else if (isCitizen) {
		Data_CityInfo.dieSoundCitizen++;
		if (Data_CityInfo.dieSoundCitizen >= 4) {
			Data_CityInfo.dieSoundCitizen = 0;
		}
		sound_effect_play(SOUND_EFFECT_CITIZEN_DIE + Data_CityInfo.dieSoundCitizen);
	}
	if (FigureIsEnemy(figureType)) {
		if (Data_CityInfo.numEnemiesInCity == 1) {
			sound_speech_play_file("wavs/army_war_cry.wav");
		}
	} else if (FigureIsLegion(figureType)) {
		if (Data_CityInfo.numSoldiersInCity == 1) {
			sound_speech_play_file("wavs/barbarian_war_cry.wav");
		}
	}
}

#define PLAY_HIT_SOUND(t, ch) \
	Data_CityInfo.t--;\
	if (Data_CityInfo.t <= 0) {\
		Data_CityInfo.t = 8;\
		sound_effect_play(ch);\
	}

void Figure_playHitSound(int figureType)
{
	switch (figureType) {
		case FIGURE_FORT_LEGIONARY:
		case FIGURE_ENEMY_CAESAR_LEGIONARY:
			PLAY_HIT_SOUND(soundHitSoldier, SOUND_EFFECT_SWORD);
			break;
		case FIGURE_FORT_MOUNTED:
		case FIGURE_ENEMY45_SWORD:
		case FIGURE_ENEMY48_CHARIOT:
		case FIGURE_ENEMY50_SWORD:
		case FIGURE_ENEMY52_MOUNTED_ARCHER:
		case FIGURE_ENEMY54_GLADIATOR:
			PLAY_HIT_SOUND(soundHitSoldier, SOUND_EFFECT_SWORD_SWING);
			break;
		case FIGURE_FORT_JAVELIN:
			PLAY_HIT_SOUND(soundHitSoldier, SOUND_EFFECT_LIGHT_SWORD);
			break;
		case FIGURE_ENEMY43_SPEAR:
		case FIGURE_ENEMY51_SPEAR:
			PLAY_HIT_SOUND(soundHitSpear, SOUND_EFFECT_SPEAR);
			break;
		case FIGURE_ENEMY44_SWORD:
		case FIGURE_ENEMY49_FAST_SWORD:
			PLAY_HIT_SOUND(soundHitClub, SOUND_EFFECT_CLUB);
			break;
		case FIGURE_ENEMY53_AXE:
			PLAY_HIT_SOUND(soundHitAxe, SOUND_EFFECT_AXE);
			break;
		case FIGURE_ENEMY46_CAMEL:
			sound_effect_play(SOUND_EFFECT_CAMEL);
			break;
		case FIGURE_ENEMY47_ELEPHANT:
			if (Data_CityInfo.soundHitElephant == 1) {
				sound_effect_play(SOUND_EFFECT_ELEPHANT_HIT);
				Data_CityInfo.soundHitElephant = 0;
			} else {
				sound_effect_play(SOUND_EFFECT_ELEPHANT);
				Data_CityInfo.soundHitElephant = 1;
			}
			break;
		case FIGURE_LION_TAMER:
			sound_effect_play(SOUND_EFFECT_LION_ATTACK);
			break;
		case FIGURE_WOLF:
			Data_CityInfo.soundHitWolf--;
			if (Data_CityInfo.soundHitWolf <= 0) {
				Data_CityInfo.soundHitWolf = 4;
				sound_effect_play(SOUND_EFFECT_WOLF_ATTACK);
			}
			break;
	}
}
