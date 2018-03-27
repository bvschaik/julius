#include "sound.h"

#include "city/data_private.h"

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
    return update_field(&city_data.sound.march_enemy, 200);
}

int city_sound_update_march_horse()
{
    return update_field(&city_data.sound.march_horse, 200);
}

int city_sound_update_march_wolf()
{
    return update_field(&city_data.sound.march_wolf, 12);
}

int city_sound_update_shoot_arrow()
{
    return update_field(&city_data.sound.shoot_arrow, 10);
}

int city_sound_update_hit_wolf()
{
    return update_field(&city_data.sound.hit_wolf, 4);
}

int city_sound_update_hit_soldier()
{
    return update_field(&city_data.sound.hit_soldier, 8);
}

int city_sound_update_hit_axe()
{
    return update_field(&city_data.sound.hit_axe, 8);
}

int city_sound_update_hit_club()
{
    return update_field(&city_data.sound.hit_club, 8);
}

int city_sound_update_hit_spear()
{
    return update_field(&city_data.sound.hit_spear, 8);
}

int city_sound_update_hit_elephant()
{
    if (city_data.sound.hit_elephant == 1) {
        city_data.sound.hit_elephant = 0;
    } else {
        city_data.sound.hit_elephant = 1;
    }
    return city_data.sound.hit_elephant;
}

int city_sound_update_die_citizen()
{
    city_data.sound.die_citizen++;
    if (city_data.sound.die_citizen >= 4) {
        city_data.sound.die_citizen = 0;
    }
    return city_data.sound.die_citizen;
}

int city_sound_update_die_soldier()
{
    city_data.sound.die_soldier++;
    if (city_data.sound.die_soldier >= 4) {
        city_data.sound.die_soldier = 0;
    }
    return city_data.sound.die_soldier;
}
