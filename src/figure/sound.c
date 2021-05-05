#include "sound.h"

#include "city/figures.h"
#include "city/sound.h"
#include "sound/effect.h"
#include "sound/speech.h"

void figure_play_die_sound(const figure *f)
{
    int is_soldier = 0;
    int is_citizen = 0;
    int is_female_citizen = 0;
    switch (f->type) {
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
        case FIGURE_FORT_MOUNTED:
        case FIGURE_ENEMY_CAESAR_MOUNTED:
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
        case FIGURE_FORT_LEGIONARY:
        case FIGURE_GLADIATOR:
        case FIGURE_INDIGENOUS_NATIVE:
        case FIGURE_TOWER_SENTRY:
        case FIGURE_MESS_HALL_SUPPLIER:
        case FIGURE_WATCHMAN:
        case FIGURE_ENEMY43_SPEAR:
        case FIGURE_ENEMY44_SWORD:
        case FIGURE_ENEMY45_SWORD:
        case FIGURE_ENEMY49_FAST_SWORD:
        case FIGURE_ENEMY50_SWORD:
        case FIGURE_ENEMY51_SPEAR:
        case FIGURE_ENEMY53_AXE:
        case FIGURE_ENEMY54_GLADIATOR:
        case FIGURE_ENEMY_CAESAR_JAVELIN:
        case FIGURE_ENEMY_CAESAR_LEGIONARY:
            is_soldier = 1;
            break;
        case FIGURE_MARKET_TRADER:
        case FIGURE_MARKET_SUPPLIER:
        case FIGURE_BATHHOUSE_WORKER:
        case FIGURE_BARKEEP:
            is_female_citizen = 1;
            break;
        default:
            is_citizen = 1;
            break;
    }
    if (is_soldier) {
        sound_effect_play(SOUND_EFFECT_SOLDIER_DIE + city_sound_update_die_soldier());
    } else if (is_female_citizen) {
        sound_effect_play(SOUND_EFFECT_FEMALE_CITIZEN_DIE);
    } else if (is_citizen) {
        sound_effect_play(SOUND_EFFECT_CITIZEN_DIE + city_sound_update_die_citizen());
    }
    if (figure_is_enemy(f)) {
        if (city_figures_enemies() == 1) {
            sound_speech_play_file("wavs/army_war_cry.wav");
        }
    } else if (figure_is_legion(f)) {
        if (city_figures_soldiers() == 1) {
            sound_speech_play_file("wavs/barbarian_war_cry.wav");
        }
    }
}

void figure_play_hit_sound(figure_type type)
{
    switch (type) {
        case FIGURE_FORT_LEGIONARY:
        case FIGURE_ENEMY_CAESAR_LEGIONARY:
            if (city_sound_update_hit_soldier()) {
                sound_effect_play(SOUND_EFFECT_SWORD);
            }
            break;
        case FIGURE_FORT_MOUNTED:
        case FIGURE_ENEMY45_SWORD:
        case FIGURE_ENEMY48_CHARIOT:
        case FIGURE_ENEMY50_SWORD:
        case FIGURE_ENEMY52_MOUNTED_ARCHER:
        case FIGURE_ENEMY54_GLADIATOR:
            if (city_sound_update_hit_soldier()) {
                sound_effect_play(SOUND_EFFECT_SWORD_SWING);
            }
            break;
        case FIGURE_FORT_JAVELIN:
            if (city_sound_update_hit_soldier()) {
                sound_effect_play(SOUND_EFFECT_LIGHT_SWORD);
            }
            break;
        case FIGURE_ENEMY43_SPEAR:
        case FIGURE_ENEMY51_SPEAR:
            if (city_sound_update_hit_spear()) {
                sound_effect_play(SOUND_EFFECT_SPEAR);
            }
            break;
        case FIGURE_ENEMY44_SWORD:
        case FIGURE_ENEMY49_FAST_SWORD:
            if (city_sound_update_hit_club()) {
                sound_effect_play(SOUND_EFFECT_CLUB);
            }
            break;
        case FIGURE_ENEMY53_AXE:
            if (city_sound_update_hit_axe()) {
                sound_effect_play(SOUND_EFFECT_AXE);
            }
            break;
        case FIGURE_ENEMY46_CAMEL:
            sound_effect_play(SOUND_EFFECT_CAMEL);
            break;
        case FIGURE_ENEMY47_ELEPHANT:
            if (city_sound_update_hit_elephant()) {
                sound_effect_play(SOUND_EFFECT_ELEPHANT);
            } else {
                sound_effect_play(SOUND_EFFECT_ELEPHANT_HIT);
            }
            break;
        case FIGURE_LION_TAMER:
            sound_effect_play(SOUND_EFFECT_LION_ATTACK);
            break;
        case FIGURE_WOLF:
            if (city_sound_update_hit_wolf()) {
                sound_effect_play(SOUND_EFFECT_WOLF_ATTACK);
            }
            break;
        default:
            break;
    }
}
