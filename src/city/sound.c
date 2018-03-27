#include "sound.h"

#include "Data/CityInfo.h"

static int update_field(int8_t *field, int max_value)
{
    (*field)--;
    if (*field <= 0) {
        *field = (int8_t) max_value;
        return 1;
    } else {
        return 0;
    }
}

int city_sound_update_march_enemy()
{
    return update_field(&Data_CityInfo.soundMarchEnemy, 200);
}

int city_sound_update_march_horse()
{
    return update_field(&Data_CityInfo.soundMarchHorse, 200);
}

int city_sound_update_march_wolf()
{
    return update_field(&Data_CityInfo.soundMarchWolf, 12);
}

int city_sound_update_shoot_arrow()
{
    return update_field(&Data_CityInfo.soundShootArrow, 10);
}

int city_sound_update_hit_wolf()
{
    return update_field(&Data_CityInfo.soundHitWolf, 4);
}

int city_sound_update_hit_soldier()
{
    return update_field(&Data_CityInfo.soundHitSoldier, 8);
}

int city_sound_update_hit_axe()
{
    return update_field(&Data_CityInfo.soundHitAxe, 8);
}

int city_sound_update_hit_club()
{
    return update_field(&Data_CityInfo.soundHitClub, 8);
}

int city_sound_update_hit_spear()
{
    return update_field(&Data_CityInfo.soundHitSpear, 8);
}

int city_sound_update_hit_elephant()
{
    if (Data_CityInfo.soundHitElephant == 1) {
        Data_CityInfo.soundHitElephant = 0;
    } else {
        Data_CityInfo.soundHitElephant = 1;
    }
    return Data_CityInfo.soundHitElephant;
}

int city_sound_update_die_citizen()
{
    Data_CityInfo.dieSoundCitizen++;
    if (Data_CityInfo.dieSoundCitizen >= 4) {
        Data_CityInfo.dieSoundCitizen = 0;
    }
    return Data_CityInfo.dieSoundCitizen;
}

int city_sound_update_die_soldier()
{
    Data_CityInfo.dieSoundSoldier++;
    if (Data_CityInfo.dieSoundSoldier >= 4) {
        Data_CityInfo.dieSoundSoldier = 0;
    }
    return Data_CityInfo.dieSoundSoldier;
}
