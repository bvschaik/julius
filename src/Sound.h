#ifndef SOUND_H
#define SOUND_H

enum {
	SoundDirectionLeft = 0,
	SoundDirectionCenter = 2,
	SoundDirectionRight = 4
};

enum {
	SoundChannel_Speech = 0,

	// user interface effects
	SoundChannel_EffectsMin = 1,
	SoundChannel_EffectsMax = 44,

	SoundChannel_Panel1 = 1,
	SoundChannel_Sidebar = 2, // 2, panel3
	SoundChannel_Icon = 3, // 3, icon1 (click)
	SoundChannel_Build = 4, // 4 build1
	SoundChannel_Explosion = 5, // 5
	SoundChannel_Fanfare = 6, // 6, fanfare
	SoundChannel_FanfareUrgent = 7, // 7, fanfare2
	// battle effects
	SoundChannel_Arrow = 8, // 10
	SoundChannel_ArrowHit = 9, // 11
	SoundChannel_Axe = 10, // 12
	SoundChannel_BallistaShoot = 11, // 13
	SoundChannel_BallistaHitGround = 12, // 14
	SoundChannel_BallistaHitPerson = 13, // 15
	SoundChannel_Club = 14, // 16
	SoundChannel_Camel = 15, // 17
	SoundChannel_Elephant = 16, // 18
	SoundChannel_ElephantHit = 17, // 19
	SoundChannel_ElephantDie = 18, // 20
	SoundChannel_Horse = 19, // 21
	SoundChannel_Horse2 = 20, // 22
	SoundChannel_HorseMoving = 21, // 23
	SoundChannel_Javelin = 22, // 24
	SoundChannel_LionAttack = 23, // 25
	SoundChannel_LionDie = 24, // 26
	SoundChannel_Marching = 25, // 27
	SoundChannel_Sword = 26, // 140
	SoundChannel_SwordSwing = 27, // 141
	SoundChannel_SwordLight = 28, // 142
	SoundChannel_Spear = 29, // 143
	SoundChannel_WolfAttack = 30, // 144
	SoundChannel_WolfDie = 31, // 145
	SoundChannel_SoldierDie = 32, // 33,34,35 (was:146-149) die1, die2, die4, die10
	SoundChannel_CitizenDie = 36, // 37,38,39 (was: 150-153) die3, die5, die8, die9
	SoundChannel_SheepDie = 40, // 154
	SoundChannel_ZebraDie = 41, // 155
	SoundChannel_WolfHowl = 42, // 156
	SoundChannel_FireSplash = 43, // 157
	SoundChannel_FormationShield = 44, // 158

	// city sounds (from buildings)
	SoundChannel_CityOffset = 15,
	SoundChannel_CityMin = 45,
	SoundChannel_CityMax = 148,

	SoundChannel_City_HouseSlum = 30,
	SoundChannel_City_HousePoor = 34,
	SoundChannel_City_HouseMedium = 38,
	SoundChannel_City_HouseGood = 42,
	SoundChannel_City_HousePosh = 46,
	SoundChannel_City_Amphitheater = 50,
	SoundChannel_City_Theater = 51,
	SoundChannel_City_Hippodrome = 52,
	SoundChannel_City_Colosseum = 53,
	SoundChannel_City_GladiatorSchool = 54,
	SoundChannel_City_LionPit = 55,
	SoundChannel_City_ActorColony = 56,
	SoundChannel_City_ChariotMaker = 57,
	SoundChannel_City_Garden = 58,
	SoundChannel_City_Clinic = 62,
	SoundChannel_City_Hospital = 63,
	SoundChannel_City_Bathhouse = 64,
	SoundChannel_City_Barber = 65,
	SoundChannel_City_School = 66,
	SoundChannel_City_Academy = 67,
	SoundChannel_City_Library = 68,
	SoundChannel_City_Prefecture = 69,
	SoundChannel_City_Fort = 70,
	SoundChannel_City_Tower = 74,
	SoundChannel_City_TempleCeres = 78,
	SoundChannel_City_TempleNeptune = 79,
	SoundChannel_City_TempleMercury = 80,
	SoundChannel_City_TempleMars = 81,
	SoundChannel_City_TempleVenus = 82,
	SoundChannel_City_Market = 83,
	SoundChannel_City_Granary = 87,
	SoundChannel_City_Warehouse = 89,
	SoundChannel_City_Shipyard = 91,
	SoundChannel_City_Dock = 93,
	SoundChannel_City_Wharf = 95,
	SoundChannel_City_Palace = 97,
	SoundChannel_City_EngineersPost = 98,
	SoundChannel_City_Senate = 99,
	SoundChannel_City_Forum = 100,
	SoundChannel_City_Reservoir = 101,
	SoundChannel_City_Fountain = 102,
	SoundChannel_City_Well = 106,
	SoundChannel_City_MilitaryAcademy = 110,
	SoundChannel_City_Oracle = 111,
	SoundChannel_City_BurningRuin = 112,
	SoundChannel_City_WheatFarm = 113,
	SoundChannel_City_VegetableFarm = 114,
	SoundChannel_City_FruitFarm = 115,
	SoundChannel_City_OliveFarm = 116,
	SoundChannel_City_VineFarm = 117,
	SoundChannel_City_PigFarm = 118,
	SoundChannel_City_Quarry = 119,
	SoundChannel_City_IronMine = 120,
	SoundChannel_City_TimberYard = 121,
	SoundChannel_City_ClayPit = 122,
	SoundChannel_City_WineWorkshop = 123,
	SoundChannel_City_OilWorkshop = 124,
	SoundChannel_City_WeaponsWorkshop = 125,
	SoundChannel_City_FurnitureWorkshop = 126,
	SoundChannel_City_PotteryWorkshop = 127,
	SoundChannel_City_EmptyLand = 128,
	SoundChannel_City_River = 132,
	SoundChannel_City_MissionPost = 133,

	SoundChannel_TotalChannels = SoundChannel_City_MissionPost + SoundChannel_CityOffset + 1
};

void Sound_init();
void Sound_shutdown();

void Sound_setMusicVolume(int percentage);
void Sound_setSpeechVolume(int percentage);
void Sound_setEffectsVolume(int percentage);
void Sound_setCityVolume(int percentage);

void Sound_stopMusic();
void Sound_stopSpeech();

// NB: only internal use!
void Sound_playCityChannel_internal(int channel, int direction);

void Sound_City_init();
void Sound_City_markBuildingView(int buildingId, int direction);
void Sound_City_decayViews();
void Sound_City_play();

void Sound_Music_reset();
void Sound_Music_update();

void Sound_Effects_playChannel(int channel);

void Sound_Speech_playFile(const char *filename);

#endif
