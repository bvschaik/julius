#include "phrase.h"

#include "building/building.h"
#include "building/market.h"
#include "city/constants.h"
#include "city/culture.h"
#include "city/figures.h"
#include "city/gods.h"
#include "city/labor.h"
#include "city/population.h"
#include "city/resource.h"
#include "city/sentiment.h"
#include "core/calc.h"
#include "figure/trader.h"
#include "figuretype/trader.h"
#include "sound/speech.h"

#include <string.h>

#define SOUND_FILENAME_MAX 32

static const char FIGURE_SOUNDS[32][20][SOUND_FILENAME_MAX] = {
    { // 0
        "vigils_starv1.wav", "vigils_nojob1.wav", "vigils_needjob1.wav", "vigils_nofun1.wav",
        "vigils_relig1.wav", "vigils_great1.wav", "vigils_great2.wav", "vigils_exact1.wav",
        "vigils_exact2.wav", "vigils_exact3.wav", "vigils_exact4.wav", "vigils_exact5.wav",
        "vigils_exact6.wav", "vigils_exact7.wav", "vigils_exact8.wav", "vigils_exact9.wav",
        "vigils_exact10.wav", "vigils_free1.wav", "vigils_free2.wav", "vigils_free3.wav"
    },
    { // 1
        "wallguard_starv1.wav", "wallguard_nojob1.wav", "wallguard_needjob1.wav", "wallguard_nofun1.wav",
        "wallguard_relig1.wav", "wallguard_great1.wav", "wallguard_great2.wav", "wallguard_exact1.wav",
        "wallguard_exact2.wav", "wallguard_exact3.wav", "wallguard_exact4.wav", "wallguard_exact5.wav",
        "wallguard_exact6.wav", "wallguard_exact7.wav", "wallguard_exact8.wav", "wallguard_exact9.wav",
        "wallguard_exact0.wav", "wallguard_free1.wav", "wallguard_free2.wav", "wallguard_free3.wav"
    },
    { // 2
        "engine_starv1.wav", "engine_nojob1.wav", "engine_needjob1.wav", "engine_nofun1.wav",
        "engine_relig1.wav", "engine_great1.wav", "engine_great2.wav", "engine_exact1.wav",
        "engine_exact2.wav", "engine_exact3.wav", "engine_exact4.wav", "engine_exact5.wav",
        "engine_exact6.wav", "engine_exact7.wav", "engine_exact8.wav", "engine_exact9.wav",
        "engine_exact0.wav", "engine_free1.wav", "engine_free2.wav", "engine_free3.wav"
    },
    { // 3
        "taxman_starv1.wav", "taxman_nojob1.wav", "taxman_needjob1.wav", "taxman_nofun1.wav",
        "taxman_relig1.wav", "taxman_great1.wav", "taxman_great2.wav", "taxman_exact1.wav",
        "taxman_exact2.wav", "taxman_exact3.wav", "taxman_exact4.wav", "taxman_exact5.wav",
        "taxman_exact6.wav", "taxman_exact7.wav", "taxman_exact8.wav", "taxman_exact9.wav",
        "taxman_exact0.wav", "taxman_free1.wav", "taxman_free2.wav", "taxman_free3.wav"
    },
    { // 4
        "market_starv1.wav", "market_nojob1.wav", "market_needjob1.wav", "market_nofun1.wav",
        "market_relig1.wav", "market_great1.wav", "market_great2.wav", "market_exact2.wav",
        "market_exact1.wav", "market_exact3.wav", "market_exact4.wav", "market_exact5.wav",
        "market_exact6.wav", "market_exact7.wav", "market_exact8.wav", "market_exact9.wav",
        "market_exact0.wav", "market_free1.wav", "market_free2.wav", "market_free3.wav"
    },
    { // 5
        "crtpsh_starv1.wav", "crtpsh_nojob1.wav", "crtpsh_needjob1.wav", "crtpsh_nofun1.wav",
        "crtpsh_relig1.wav", "crtpsh_great1.wav", "crtpsh_great2.wav", "crtpsh_exact1.wav",
        "crtpsh_exact2.wav", "crtpsh_exact3.wav", "crtpsh_exact4.wav", "crtpsh_exact5.wav",
        "crtpsh_exact6.wav", "crtpsh_exact7.wav", "crtpsh_exact8.wav", "crtpsh_exact9.wav",
        "crtpsh_exact0.wav", "crtpsh_free1.wav", "crtpsh_free2.wav", "crtpsh_free3.wav"
    },
    { // 6
        "donkey_starv1.wav", "donkey_nojob1.wav", "donkey_needjob1.wav", "donkey_nofun1.wav",
        "donkey_relig1.wav", "donkey_great1.wav", "donkey_great2.wav", "donkey_exact1.wav",
        "donkey_exact2.wav", "donkey_exact3.wav", "donkey_exact4.wav", "donkey_exact5.wav",
        "donkey_exact6.wav", "donkey_exact7.wav", "donkey_exact8.wav", "donkey_exact9.wav",
        "donkey_exact0.wav", "donkey_free1.wav", "donkey_free2.wav", "donkey_free3.wav"
    },
    { // 7
        "boats_starv1.wav", "boats_nojob1.wav", "boats_needjob1.wav", "boats_nofun1.wav",
        "boats_relig1.wav", "boats_great1.wav", "boats_great2.wav", "boats_exact2.wav",
        "boats_exact1.wav", "boats_exact3.wav", "boats_exact4.wav", "boats_exact5.wav",
        "boats_exact6.wav", "boats_exact7.wav", "boats_exact8.wav", "boats_exact9.wav",
        "boats_exact0.wav", "boats_free1.wav", "boats_free2.wav", "boats_free3.wav"
    },
    { // 8
        "priest_starv1.wav", "priest_nojob1.wav", "priest_needjob1.wav", "priest_nofun1.wav",
        "priest_relig1.wav", "priest_great1.wav", "priest_great2.wav", "priest_exact1.wav",
        "priest_exact2.wav", "priest_exact3.wav", "priest_exact4.wav", "priest_exact5.wav",
        "priest_exact6.wav", "priest_exact7.wav", "priest_exact8.wav", "priest_exact9.wav",
        "priest_exact0.wav", "priest_free1.wav", "priest_free2.wav", "priest_free3.wav"
    },
    { // 9
        "teach_starv1.wav", "teach_nojob1.wav", "teach_needjob1.wav", "teach_nofun1.wav",
        "teach_relig1.wav", "teach_great1.wav", "teach_great2.wav", "teach_exact1.wav",
        "teach_exact2.wav", "teach_exact3.wav", "teach_exact4.wav", "teach_exact5.wav",
        "teach_exact6.wav", "teach_exact7.wav", "teach_exact8.wav", "teach_exact9.wav",
        "teach_exact0.wav", "teach_free1.wav", "teach_free2.wav", "teach_free3.wav"
    },
    { // 10
        "pupils_starv1.wav", "pupils_nojob1.wav", "pupils_needjob1.wav", "pupils_nofun1.wav",
        "pupils_relig1.wav", "pupils_great1.wav", "pupils_great2.wav", "pupils_exact1.wav",
        "pupils_exact2.wav", "pupils_exact3.wav", "pupils_exact4.wav", "pupils_exact5.wav",
        "pupils_exact6.wav", "pupils_exact7.wav", "pupils_exact8.wav", "pupils_exact9.wav",
        "pupils_exact0.wav", "pupils_free1.wav", "pupils_free2.wav", "pupils_free3.wav"
    },
    { // 11
        "bather_starv1.wav", "bather_nojob1.wav", "bather_needjob1.wav", "bather_nofun1.wav",
        "bather_relig1.wav", "bather_great1.wav", "bather_great2.wav", "bather_exact1.wav",
        "bather_exact2.wav", "bather_exact3.wav", "bather_exact4.wav", "bather_exact5.wav",
        "bather_exact6.wav", "bather_exact7.wav", "bather_exact8.wav", "bather_exact9.wav",
        "bather_exact0.wav", "bather_free1.wav", "bather_free2.wav", "bather_free3.wav"
    },
    { // 12
        "doctor_starv1.wav", "doctor_nojob1.wav", "doctor_needjob1.wav", "doctor_nofun1.wav",
        "doctor_relig1.wav", "doctor_great1.wav", "doctor_great2.wav", "doctor_exact1.wav",
        "doctor_exact2.wav", "doctor_exact3.wav", "doctor_exact4.wav", "doctor_exact5.wav",
        "doctor_exact6.wav", "doctor_exact7.wav", "doctor_exact8.wav", "doctor_exact9.wav",
        "doctor_exact0.wav", "doctor_free1.wav", "doctor_free2.wav", "doctor_free3.wav"
    },
    { // 13
        "barber_starv1.wav", "barber_nojob1.wav", "barber_needjob1.wav", "barber_nofun1.wav",
        "barber_relig1.wav", "barber_great1.wav", "barber_great2.wav", "barber_exact1.wav",
        "barber_exact2.wav", "barber_exact3.wav", "barber_exact4.wav", "barber_exact5.wav",
        "barber_exact6.wav", "barber_exact7.wav", "barber_exact8.wav", "barber_exact9.wav",
        "barber_exact0.wav", "barber_free1.wav", "barber_free2.wav", "barber_free3.wav"
    },
    { // 14
        "actors_starv1.wav", "actors_nojob1.wav", "actors_needjob1.wav", "actors_nofun1.wav",
        "actors_relig1.wav", "actors_great1.wav", "actors_great2.wav", "actors_exact1.wav",
        "actors_exact2.wav", "actors_exact3.wav", "actors_exact4.wav", "actors_exact5.wav",
        "actors_exact6.wav", "actors_exact7.wav", "actors_exact8.wav", "actors_exact9.wav",
        "actors_exact0.wav", "actors_free1.wav", "actors_free2.wav", "actors_free3.wav"
    },
    { // 15
        "gladtr_starv1.wav", "gladtr_nojob1.wav", "gladtr_needjob1.wav", "gladtr_nofun1.wav",
        "gladtr_relig1.wav", "gladtr_great1.wav", "gladtr_great2.wav", "gladtr_exact1.wav",
        "gladtr_exact2.wav", "gladtr_exact3.wav", "gladtr_exact4.wav", "gladtr_exact5.wav",
        "gladtr_exact6.wav", "gladtr_exact7.wav", "gladtr_exact8.wav", "gladtr_exact9.wav",
        "gladtr_exact0.wav", "gladtr_free1.wav", "gladtr_free2.wav", "gladtr_free3.wav"
    },
    { // 16
        "liontr_starv1.wav", "liontr_nojob1.wav", "liontr_needjob1.wav", "liontr_nofun1.wav",
        "liontr_relig1.wav", "liontr_great1.wav", "liontr_great2.wav", "liontr_exact1.wav",
        "liontr_exact2.wav", "liontr_exact3.wav", "liontr_exact4.wav", "liontr_exact5.wav",
        "liontr_exact6.wav", "liontr_exact7.wav", "liontr_exact8.wav", "liontr_exact9.wav",
        "liontr_exact0.wav", "liontr_free1.wav", "liontr_free2.wav", "liontr_free3.wav"
    },
    { // 17
        "charot_starv1.wav", "charot_nojob1.wav", "charot_needjob1.wav", "charot_nofun1.wav",
        "charot_relig1.wav", "charot_great1.wav", "charot_great2.wav", "charot_exact1.wav",
        "charot_exact2.wav", "charot_exact3.wav", "charot_exact4.wav", "charot_exact5.wav",
        "charot_exact6.wav", "charot_exact7.wav", "charot_exact8.wav", "charot_exact9.wav",
        "charot_exact0.wav", "charot_free1.wav", "charot_free2.wav", "charot_free3.wav"
    },
    { // 18
        "patric_starv1.wav", "patric_nojob1.wav", "patric_needjob1.wav", "patric_nofun1.wav",
        "patric_relig1.wav", "patric_great1.wav", "patric_great2.wav", "patric_exact1.wav",
        "patric_exact2.wav", "patric_exact3.wav", "patric_exact4.wav", "patric_exact5.wav",
        "patric_exact6.wav", "patric_exact7.wav", "patric_exact8.wav", "patric_exact9.wav",
        "patric_exact0.wav", "patric_free1.wav", "patric_free2.wav", "patric_free3.wav"
    },
    { // 19
        "pleb_starv1.wav", "pleb_nojob1.wav", "pleb_needjob1.wav", "pleb_nofun1.wav",
        "pleb_relig1.wav", "pleb_great1.wav", "pleb_great2.wav", "pleb_exact1.wav",
        "pleb_exact2.wav", "pleb_exact3.wav", "pleb_exact4.wav", "pleb_exact5.wav",
        "pleb_exact6.wav", "pleb_exact7.wav", "pleb_exact8.wav", "pleb_exact9.wav",
        "pleb_exact0.wav", "pleb_free1.wav", "pleb_free2.wav", "pleb_free3.wav"
    },
    { // 20
        "rioter_starv1.wav", "rioter_nojob1.wav", "rioter_needjob1.wav", "rioter_nofun1.wav",
        "rioter_relig1.wav", "rioter_great1.wav", "rioter_great2.wav", "rioter_exact1.wav",
        "rioter_exact2.wav", "rioter_exact3.wav", "rioter_exact4.wav", "rioter_exact5.wav",
        "rioter_exact6.wav", "rioter_exact7.wav", "rioter_exact8.wav", "rioter_exact9.wav",
        "rioter_exact0.wav", "rioter_free1.wav", "rioter_free2.wav", "rioter_free3.wav"
    },
    { // 21
        "homeless_starv1.wav", "homeless_nojob1.wav", "homeless_needjob1.wav", "homeless_nofun1.wav",
        "homeless_relig1.wav", "homeless_great1.wav", "homeless_great2.wav", "homeless_exact1.wav",
        "homeless_exact2.wav", "homeless_exact3.wav", "homeless_exact4.wav", "homeless_exact5.wav",
        "homeless_exact6.wav", "homeless_exact7.wav", "homeless_exact8.wav", "homeless_exact9.wav",
        "homeless_exact0.wav", "homeless_free1.wav", "homeless_free2.wav", "homeless_free3.wav"
    },
    { // 22
        "unemploy_starv1.wav", "unemploy_nojob1.wav", "unemploy_needjob1.wav", "unemploy_nofun1.wav",
        "unemploy_relig1.wav", "unemploy_great1.wav", "unemploy_great2.wav", "unemploy_exact1.wav",
        "unemploy_exact2.wav", "unemploy_exact3.wav", "unemploy_exact4.wav", "unemploy_exact5.wav",
        "unemploy_exact6.wav", "unemploy_exact7.wav", "unemploy_exact8.wav", "unemploy_exact9.wav",
        "unemploy_exact0.wav", "unemploy_free1.wav", "unemploy_free2.wav", "unemploy_free3.wav"
    },
    { // 23
        "emigrate_starv1.wav", "emigrate_nojob1.wav", "emigrate_needjob1.wav", "emigrate_nofun1.wav",
        "emigrate_relig1.wav", "emigrate_great1.wav", "emigrate_great2.wav", "emigrate_exact1.wav",
        "emigrate_exact2.wav", "emigrate_exact3.wav", "emigrate_exact4.wav", "emigrate_exact5.wav",
        "emigrate_exact6.wav", "emigrate_exact7.wav", "emigrate_exact8.wav", "emigrate_exact9.wav",
        "emigrate_exact0.wav", "emigrate_free1.wav", "emigrate_free2.wav", "emigrate_free3.wav"
    },
    { // 24
        "immigrant_starv1.wav", "immigrant_nojob1.wav", "immigrant_needjob1.wav", "immigrant_nofun1.wav",
        "immigrant_relig1.wav", "immigrant_great1.wav", "immigrant_great2.wav", "immigrant_exact1.wav",
        "immigrant_exact2.wav", "immigrant_exact3.wav", "immigrant_exact4.wav", "immigrant_exact5.wav",
        "immigrant_exact6.wav", "immigrant_exact7.wav", "immigrant_exact8.wav", "immigrant_exact9.wav",
        "immigrant_exact0.wav", "immigrant_free1.wav", "immigrant_free2.wav", "immigrant_free3.wav"
    },
    { // 25
        "enemy_starv1.wav", "enemy_nojob1.wav", "enemy_needjob1.wav", "enemy_nofun1.wav",
        "enemy_relig1.wav", "enemy_great1.wav", "enemy_great2.wav", "enemy_exact1.wav",
        "enemy_exact2.wav", "enemy_exact3.wav", "enemy_exact4.wav", "enemy_exact5.wav",
        "enemy_exact6.wav", "enemy_exact7.wav", "enemy_exact8.wav", "enemy_exact9.wav",
        "enemy_exact0.wav", "enemy_free1.wav", "enemy_free2.wav", "enemy_free3.wav"
    },
    { // 26
        "local_starv1.wav", "local_nojob1.wav", "local_needjob1.wav", "local_nofun1.wav",
        "local_relig1.wav", "local_great1.wav", "local_great2.wav", "local_exact1.wav",
        "local_exact2.wav", "local_exact3.wav", "local_exact4.wav", "local_exact5.wav",
        "local_exact6.wav", "local_exact7.wav", "local_exact8.wav", "local_exact9.wav",
        "local_exact0.wav", "local_free1.wav", "local_free2.wav", "local_free3.wav"
    },
    { // 27
        "libary_starv1.wav", "libary_nojob1.wav", "libary_needjob1.wav", "libary_nofun1.wav",
        "libary_relig1.wav", "libary_great1.wav", "libary_great2.wav", "libary_exact1.wav",
        "libary_exact2.wav", "libary_exact3.wav", "libary_exact4.wav", "libary_exact5.wav",
        "libary_exact6.wav", "libary_exact7.wav", "libary_exact8.wav", "libary_exact9.wav",
        "libary_exact0.wav", "libary_free1.wav", "libary_free2.wav", "libary_free3.wav"
    },
    { // 28
        "srgeon_starv1.wav", "srgeon_nojob1.wav", "srgeon_needjob1.wav", "srgeon_nofun1.wav",
        "srgeon_relig1.wav", "srgeon_great1.wav", "srgeon_great2.wav", "srgeon_exact1.wav",
        "srgeon_exact2.wav", "srgeon_exact3.wav", "srgeon_exact4.wav", "srgeon_exact5.wav",
        "srgeon_exact6.wav", "srgeon_exact7.wav", "srgeon_exact8.wav", "srgeon_exact9.wav",
        "srgeon_exact0.wav", "srgeon_free1.wav", "srgeon_free2.wav", "srgeon_free3.wav"
    },
    { // 29
        "docker_starv1.wav", "docker_nojob1.wav", "docker_needjob1.wav", "docker_nofun1.wav",
        "docker_relig1.wav", "docker_great1.wav", "docker_great2.wav", "docker_exact1.wav",
        "docker_exact2.wav", "docker_exact3.wav", "docker_exact4.wav", "docker_exact5.wav",
        "docker_exact6.wav", "docker_exact7.wav", "docker_exact8.wav", "docker_exact9.wav",
        "docker_exact0.wav", "docker_free1.wav", "docker_free2.wav", "docker_free3.wav"
    },
    { // 30
        "missionary_starv1.wav", "missionary_nojob1.wav", "missionary_needjob1.wav", "missionary_nofun1.wav",
        "missionary_relig1.wav", "missionary_great1.wav", "missionary_great2.wav", "missionary_exact1.wav",
        "missionary_exact2.wav", "missionary_exact3.wav", "missionary_exact4.wav", "missionary_exact5.wav",
        "missionary_exact6.wav", "missionary_exact7.wav", "missionary_exact8.wav", "missionary_exact9.wav",
        "missionary_exact0.wav", "missionary_free1.wav", "missionary_free2.wav", "missionary_free3.wav"
    },
    { // 31
        "granboy_starv1.wav", "granboy_nojob1.wav", "granboy_needjob1.wav", "granboy_nofun1.wav",
        "granboy_relig1.wav", "granboy_great1.wav", "granboy_great2.wav", "granboy_exact1.wav",
        "granboy_exact2.wav", "granboy_exact3.wav", "granboy_exact4.wav", "granboy_exact5.wav",
        "granboy_exact6.wav", "granboy_exact7.wav", "granboy_exact8.wav", "granboy_exact9.wav",
        "granboy_exact0.wav", "granboy_free1.wav", "granboy_free2.wav", "granboy_free3.wav"
    }
};

static const int FIGURE_TYPE_TO_SOUND_TYPE[] = {
    -1, 24, 23, 21, 5, 19, -1, 3, 2, 5, // 0-9
    0, 1, 1, 1, -1, 14, 15, 16, 17, 6, // 10-19
    7, 6, 20, 20, 20, -1, 4, 8, 10, 9, // 20-29
    9, 13, 11, 12, 12, 19, -1, -1, 5, 4, // 30-39
    18, -1, 1, 25, 25, 25, 25, 25, 25, 25, // 40-49
    25, 25, 25, 25, 25, 25, 25, 25, -1, -1, // 50-59
    -1, -1, -1, -1, 30, -1, 31, -1, -1, -1, // 60-69
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // 70-79
};

enum {
    GOD_STATE_NONE = 0,
    GOD_STATE_VERY_ANGRY = 1,
    GOD_STATE_ANGRY = 2
};

static void play_sound_file(int sound_id, int phrase_id)
{
    if (sound_id >= 0 && phrase_id >= 0) {
        char path[SOUND_FILENAME_MAX];
        strcpy(path, "wavs/");
        strcat(path, FIGURE_SOUNDS[sound_id][phrase_id]);
        sound_speech_play_file(path);
    }
}

int figure_phrase_play(figure *f)
{
    if (f->id <= 0) {
        return 0;
    }
    int sound_id = FIGURE_TYPE_TO_SOUND_TYPE[f->type];
    play_sound_file(sound_id, f->phrase_id);
    return sound_id;
}

static int lion_tamer_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        if (++f->phrase_sequence_exact >= 3) {
            f->phrase_sequence_exact = 0;
        }
        return 7 + f->phrase_sequence_exact;
    }
    return 0;
}

static int gladiator_phrase(figure *f)
{
    return f->action_state == FIGURE_ACTION_150_ATTACK ? 7 : 0;
}

static int tax_collector_phrase(figure *f)
{
    if (f->min_max_seen >= HOUSE_LARGE_CASA) {
        return 7;
    } else if (f->min_max_seen >= HOUSE_SMALL_HOVEL) {
        return 8;
    } else if (f->min_max_seen >= HOUSE_LARGE_TENT) {
        return 9;
    } else {
        return 0;
    }
}

static int market_trader_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_126_ROAMER_RETURNING) {
        if (building_market_get_max_food_stock(building_get(f->building_id)) <= 0) {
            return 9; // run out of goods
        }
    }
    return 0;
}

static int market_buyer_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE) {
        return 7;
    } else if (f->action_state == FIGURE_ACTION_146_MARKET_BUYER_RETURNING) {
        return 8;
    } else {
        return 0;
    }
}

static int cart_pusher_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_20_CARTPUSHER_INITIAL) {
        if (f->min_max_seen == 2) {
            return 7;
        } else if (f->min_max_seen == 1) {
            return 8;
        }
    } else if (f->action_state == FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE ||
            f->action_state == FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY ||
            f->action_state == FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP) {
        if (calc_maximum_distance(
            f->destination_x, f->destination_y, f->source_x, f->source_y) >= 25) {
            return 9; // too far
        }
    }
    return 0;
}

static int warehouseman_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE) {
        if (calc_maximum_distance(
            f->destination_x, f->destination_y, f->source_x, f->source_y) >= 25) {
            return 9; // too far
        }
    }
    return 0;
}

static int prefect_phrase(figure *f)
{
    if (++f->phrase_sequence_exact >= 4) {
        f->phrase_sequence_exact = 0;
    }
    if (f->action_state == FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE) {
        return 10;
    } else if (f->action_state == FIGURE_ACTION_75_PREFECT_AT_FIRE) {
        return 11 + (f->phrase_sequence_exact % 2);
    } else if (f->action_state == FIGURE_ACTION_150_ATTACK) {
        return 13 + f->phrase_sequence_exact;
    } else if (f->min_max_seen >= 50) {
        // alternate between "no sign of crime around here" and the regular city phrases
        if (f->phrase_sequence_exact % 2) {
            return 7;
        } else {
            return 0;
        }
    } else if (f->min_max_seen >= 10) {
        return 8;
    } else {
        return 9;
    }
}

static int engineer_phrase(figure *f)
{
    if (f->min_max_seen >= 60) {
        return 7;
    } else if (f->min_max_seen >= 10) {
        return 8;
    } else {
        return 0;
    }
}

static int citizen_phrase(figure *f)
{
    if (++f->phrase_sequence_exact >= 3) {
        f->phrase_sequence_exact = 0;
    }
    return 7 + f->phrase_sequence_exact;
}

static int house_seeker_phrase(figure *f)
{
    if (++f->phrase_sequence_exact >= 2) {
        f->phrase_sequence_exact = 0;
    }
    return 7 + f->phrase_sequence_exact;
}

static int emigrant_phrase(void)
{
    switch (city_sentiment_low_mood_cause()) {
        case LOW_MOOD_CAUSE_NO_JOBS:
            return 7;
        case LOW_MOOD_CAUSE_NO_FOOD:
            return 8;
        case LOW_MOOD_CAUSE_HIGH_TAXES:
            return 9;
        case LOW_MOOD_CAUSE_LOW_WAGES:
            return 10;
        default:
            return 11;
    }
}

static int tower_sentry_phrase(figure *f)
{
    if (++f->phrase_sequence_exact >= 2) {
        f->phrase_sequence_exact = 0;
    }
    int enemies = city_figures_enemies();
    if (!enemies) {
        return 7 + f->phrase_sequence_exact;
    } else if (enemies <= 10) {
        return 9;
    } else if (enemies <= 30) {
        return 10;
    } else {
        return 11;
    }
}

static int soldier_phrase(void)
{
    int enemies = city_figures_enemies();
    if (enemies >= 40) {
        return 11;
    } else if (enemies > 20) {
        return 10;
    } else if (enemies) {
        return 9;
    }
    return 0;
}

static int docker_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_WAREHOUSE ||
        f->action_state == FIGURE_ACTION_136_DOCKER_EXPORT_GOING_TO_WAREHOUSE) {
        if (calc_maximum_distance(
            f->destination_x, f->destination_y, f->source_x, f->source_y) >= 25) {
            return 9; // too far
        }
    }
    return 0;
}

static int trade_caravan_phrase(figure *f)
{
    if (++f->phrase_sequence_exact >= 2) {
        f->phrase_sequence_exact = 0;
    }
    if (f->action_state == FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING) {
        if (!trader_has_traded(f->trader_id)) {
            return 7; // no trade
        }
    } else if (f->action_state == FIGURE_ACTION_102_TRADE_CARAVAN_TRADING) {
        if (figure_trade_caravan_can_buy(f, f->destination_building_id, f->empire_city_id)) {
            return 11; // buying goods
        } else if (figure_trade_caravan_can_sell(f, f->destination_building_id, f->empire_city_id)) {
            return 10; // selling goods
        }
    }
    return 8 + f->phrase_sequence_exact;
}

static int trade_ship_phrase(figure *f)
{
    if (f->action_state == FIGURE_ACTION_115_TRADE_SHIP_LEAVING) {
        if (!trader_has_traded(f->trader_id)) {
            return 9; // no trade
        } else {
            return 11; // good trade
        }
    } else if (f->action_state == FIGURE_ACTION_112_TRADE_SHIP_MOORED) {
        int state = figure_trade_ship_is_trading(f);
        if (state == TRADE_SHIP_BUYING) {
            return 8; // buying goods
        } else if (state == TRADE_SHIP_SELLING) {
            return 7; // selling goods
        } else {
            return 9; // no trade
        }
    } else {
        return 10; // can't wait to trade
    }
}

static int phrase_based_on_figure_state(figure *f)
{
    switch (f->type) {
        case FIGURE_LION_TAMER:
            return lion_tamer_phrase(f);
        case FIGURE_GLADIATOR:
            return gladiator_phrase(f);
        case FIGURE_TAX_COLLECTOR:
            return tax_collector_phrase(f);
        case FIGURE_MARKET_TRADER:
            return market_trader_phrase(f);
        case FIGURE_MARKET_BUYER:
            return market_buyer_phrase(f);
        case FIGURE_CART_PUSHER:
            return cart_pusher_phrase(f);
        case FIGURE_WAREHOUSEMAN:
            return warehouseman_phrase(f);
        case FIGURE_PREFECT:
            return prefect_phrase(f);
        case FIGURE_ENGINEER:
            return engineer_phrase(f);
        case FIGURE_PROTESTER:
        case FIGURE_CRIMINAL:
        case FIGURE_RIOTER:
        case FIGURE_DELIVERY_BOY:
        case FIGURE_MISSIONARY:
            return citizen_phrase(f);
        case FIGURE_HOMELESS:
        case FIGURE_IMMIGRANT:
            return house_seeker_phrase(f);
        case FIGURE_EMIGRANT:
            return emigrant_phrase();
        case FIGURE_TOWER_SENTRY:
            return tower_sentry_phrase(f);
        case FIGURE_FORT_JAVELIN:
        case FIGURE_FORT_MOUNTED:
        case FIGURE_FORT_LEGIONARY:
            return soldier_phrase();
        case FIGURE_DOCKER:
            return docker_phrase(f);
        case FIGURE_TRADE_CARAVAN:
            return trade_caravan_phrase(f);
        case FIGURE_TRADE_CARAVAN_DONKEY:
            while (f->type == FIGURE_TRADE_CARAVAN_DONKEY && f->leading_figure_id) {
                f = figure_get(f->leading_figure_id);
            }
            return f->type == FIGURE_TRADE_CARAVAN ? trade_caravan_phrase(f) : 0;
        case FIGURE_TRADE_SHIP:
            return trade_ship_phrase(f);
    }
    return 0;
}

static int city_god_state(void)
{
    int least_god_happiness = 100;
    for (int i = 0; i < MAX_GODS; i++) {
        int happiness = city_god_happiness(i);
        if (happiness < least_god_happiness) {
            least_god_happiness = happiness;
        }
    }
    if (least_god_happiness < 20) {
        return GOD_STATE_VERY_ANGRY;
    } else if (least_god_happiness < 40) {
        return GOD_STATE_ANGRY;
    } else {
        return GOD_STATE_NONE;
    }
}

static int phrase_based_on_city_state(figure *f)
{
    f->phrase_sequence_city = 0;
    int god_state = city_god_state();
    int unemployment_pct = city_labor_unemployment_percentage();

    if (city_resource_food_supply_months() <= 0) {
        return 0;
    } else if (unemployment_pct >= 17) {
        return 1;
    } else if (city_labor_workers_needed() >= 10) {
        return 2;
    } else if (city_culture_average_entertainment() == 0) {
        return 3;
    } else if (god_state == GOD_STATE_VERY_ANGRY) {
        return 4;
    } else if (city_culture_average_entertainment() <= 10) {
        return 3;
    } else if (god_state == GOD_STATE_ANGRY) {
        return 4;
    } else if (city_culture_average_entertainment() <= 20) {
        return 3;
    } else if (city_resource_food_supply_months() >= 4 &&
            unemployment_pct <= 5 &&
            city_culture_average_health() > 0 &&
            city_culture_average_education() > 0) {
        if (city_population() < 500) {
            return 5;
        } else {
            return 6;
        }
    } else if (unemployment_pct >= 10) {
        return 1;
    } else {
        return 5;
    }
}

void figure_phrase_determine(figure *f)
{
    if (f->id <= 0) {
        return;
    }
    f->phrase_id = 0;

    if (figure_is_enemy(f) || f->type == FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_NATIVE_TRADER) {
        f->phrase_id = -1;
        return;
    }

    int phrase_id = phrase_based_on_figure_state(f);
    if (phrase_id) {
        f->phrase_id = phrase_id;
    } else {
        f->phrase_id = phrase_based_on_city_state(f);
    }
}
