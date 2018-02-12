#include "enemy.h"

#include "core/calc.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/formation.h"
#include "figure/formation_enemy.h"
#include "figure/formation_layout.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figuretype/missile.h"
#include "map/figure.h"
#include "scenario/gladiator_revolt.h"
#include "sound/effect.h"
#include "sound/speech.h"

#include "Data/CityInfo.h"

static void enemy_initial(figure *f, formation *m)
{
    map_figure_update(f);
    f->graphicOffset = 0;
    figure_route_remove(f);
    f->waitTicks--;
    if (f->waitTicks <= 0) {
        if (f->isGhost && f->indexInFormation == 0) {
            if (m->layout == FORMATION_ENEMY8) {
                sound_speech_play_file("wavs/drums.wav");
            } else if (m->layout == FORMATION_ENEMY12) {
                sound_speech_play_file("wavs/horn2.wav");
            } else {
                sound_speech_play_file("wavs/horn1.wav");
            }
        }
        f->isGhost = 0;
        if (m->recent_fight) {
            f->actionState = FIGURE_ACTION_154_ENEMY_FIGHTING;
        } else {
            f->destinationX = m->destination_x + f->formationPositionX;
            f->destinationY = m->destination_y + f->formationPositionY;
            if (calc_general_direction(f->x, f->y, f->destinationX, f->destinationY) < 8) {
                f->actionState = FIGURE_ACTION_153_ENEMY_MARCHING;
            }
        }
    }
    if (f->type == FIGURE_ENEMY43_SPEAR || f->type == FIGURE_ENEMY46_CAMEL ||
        f->type == FIGURE_ENEMY51_SPEAR || f->type == FIGURE_ENEMY52_MOUNTED_ARCHER) {
        // missile throwers
        f->waitTicksMissile++;
        int x_tile = 0, y_tile = 0;
        if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
            f->waitTicksMissile = 0;
            if (figure_combat_get_missile_target_for_enemy(f, 10, Data_CityInfo.numSoldiersInCity < 4, &x_tile, &y_tile)) {
                f->attackGraphicOffset = 1;
                f->direction = calc_missile_shooter_direction(f->x, f->y, x_tile, y_tile);
            } else {
                f->attackGraphicOffset = 0;
            }
        }
        if (f->attackGraphicOffset) {
            figure_type missile_type;
            switch (m->enemy_type) {
                case ENEMY_4_GOTH:
                case ENEMY_5_PERGAMUM:
                case ENEMY_9_EGYPTIAN:
                case ENEMY_10_CARTHAGINIAN:
                    missile_type = FIGURE_ARROW;
                    break;
                default:
                    missile_type = FIGURE_SPEAR;
                    break;
            }
            if (f->attackGraphicOffset == 1) {
                figure_create_missile(f->id, f->x, f->y, x_tile, y_tile, missile_type);
                formation_record_missile_fired(m);
            }
            if (missile_type == FIGURE_ARROW) {
                Data_CityInfo.soundShootArrow--;
                if (Data_CityInfo.soundShootArrow <= 0) {
                    Data_CityInfo.soundShootArrow = 10;
                    sound_effect_play(SOUND_EFFECT_ARROW);
                }
            }
            f->attackGraphicOffset++;
            if (f->attackGraphicOffset > 100) {
                f->attackGraphicOffset = 0;
            }
        }
    }
}

static void enemy_marching(figure *f, const formation *m)
{
    f->waitTicks--;
    if (f->waitTicks <= 0) {
        f->waitTicks = 50;
        f->destinationX = m->destination_x + f->formationPositionX;
        f->destinationY = m->destination_y + f->formationPositionY;
        if (calc_general_direction(f->x, f->y, f->destinationX, f->destinationY) == DIR_FIGURE_AT_DESTINATION) {
            f->actionState = FIGURE_ACTION_151_ENEMY_INITIAL;
            return;
        }
        f->destinationBuildingId = m->destination_building_id;
        figure_route_remove(f);
    }
    figure_movement_move_ticks(f, f->speedMultiplier);
    if (f->direction == DIR_FIGURE_AT_DESTINATION ||
        f->direction == DIR_FIGURE_REROUTE ||
        f->direction == DIR_FIGURE_LOST) {
        f->actionState = FIGURE_ACTION_151_ENEMY_INITIAL;
    }
}

static void enemy_fighting(figure *f, const formation *m)
{
    if (!m->recent_fight) {
        f->actionState = FIGURE_ACTION_151_ENEMY_INITIAL;
    }
    if (f->type != FIGURE_ENEMY46_CAMEL && f->type != FIGURE_ENEMY47_ELEPHANT) {
        if (f->type == FIGURE_ENEMY48_CHARIOT || f->type == FIGURE_ENEMY52_MOUNTED_ARCHER) {
            Data_CityInfo.soundMarchHorse--;
            if (Data_CityInfo.soundMarchHorse <= 0) {
                Data_CityInfo.soundMarchHorse = 200;
                sound_effect_play(SOUND_EFFECT_HORSE_MOVING);
            }
        } else {
            Data_CityInfo.soundMarchEnemy--;
            if (Data_CityInfo.soundMarchEnemy <= 0) {
                Data_CityInfo.soundMarchEnemy = 200;
                sound_effect_play(SOUND_EFFECT_MARCHING);
            }
        }
    }
    int target_id = f->targetFigureId;
    if (figure_is_dead(figure_get(target_id))) {
        f->targetFigureId = 0;
        target_id = 0;
    }
    if (target_id <= 0) {
        target_id = figure_combat_get_target_for_enemy(f->x, f->y);
        if (target_id) {
            figure *target = figure_get(target_id);
            f->destinationX = target->x;
            f->destinationY = target->y;
            f->targetFigureId = target_id;
            f->targetFigureCreatedSequence = target->createdSequence;
            target->targetedByFigureId = f->id;
            figure_route_remove(f);
        }
    }
    if (target_id > 0) {
        figure_movement_move_ticks(f, f->speedMultiplier);
        if (f->direction == DIR_FIGURE_AT_DESTINATION) {
            figure *target = figure_get(f->targetFigureId);
            f->destinationX = target->x;
            f->destinationY = target->y;
            figure_route_remove(f);
        } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
            f->actionState = FIGURE_ACTION_151_ENEMY_INITIAL;
            f->targetFigureId = 0;
        }
    } else {
        f->actionState = FIGURE_ACTION_151_ENEMY_INITIAL;
        f->waitTicks = 50;
    }
}

static void enemy_action(figure *f, formation *m)
{
    Data_CityInfo.numEnemiesInCity++;
    f->terrainUsage = FigureTerrainUsage_Enemy;
    f->formationPositionX = formation_layout_position_x(m->layout, f->indexInFormation);
    f->formationPositionY = formation_layout_position_y(m->layout, f->indexInFormation);

    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_148_FLEEING:
            f->destinationX = f->sourceX;
            f->destinationY = f->sourceY;
            figure_movement_move_ticks(f, f->speedMultiplier);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_151_ENEMY_INITIAL:
            enemy_initial(f, m);
            break;
        case FIGURE_ACTION_152_ENEMY_WAITING:
            map_figure_update(f);
            break;
        case FIGURE_ACTION_153_ENEMY_MARCHING:
            enemy_marching(f, m);
            break;
        case FIGURE_ACTION_154_ENEMY_FIGHTING:
            enemy_fighting(f, m);
            break;
    }
}

static int get_direction(figure *f)
{
    int dir;
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        dir = f->attackDirection;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    return figure_image_normalize_direction(dir);
}

static int get_missile_direction(figure *f, const formation *m)
{
    int dir;
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        dir = f->attackDirection;
    } else if (m->missile_fired || f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    return figure_image_normalize_direction(dir);
}

void figure_enemy43_spear_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_missile_direction(f, m);
    
    f->isEnemyGraphic = 1;
    
    switch (m->enemy_type) {
        case ENEMY_5_PERGAMUM:
        case ENEMY_6_SELEUCID:
        case ENEMY_7_ETRUSCAN:
        case ENEMY_8_GREEK:
            break;
        default:
            return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 745 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 745 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_151_ENEMY_INITIAL) {
        f->graphicId = 697 + dir + 8 * figure_image_missile_launcher_offset(f);
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 793 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 745 + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy44_sword_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    switch (m->enemy_type) {
        case ENEMY_5_PERGAMUM:
        case ENEMY_6_SELEUCID:
        case ENEMY_9_EGYPTIAN:
            break;
        default:
            return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 545 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 545 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 593 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 545 + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = 449 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy45_sword_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    switch (m->enemy_type) {
        case ENEMY_7_ETRUSCAN:
        case ENEMY_8_GREEK:
        case ENEMY_10_CARTHAGINIAN:
            break;
        default:
            return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 545 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 545 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 593 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 545 + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = 449 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy_camel_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_missile_direction(f, m);
    
    f->isEnemyGraphic = 1;
    
    if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    } else if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        f->graphicId = 601 + dir;
    } else if (f->actionState == FIGURE_ACTION_151_ENEMY_INITIAL) {
        f->graphicId = 697 + dir + 8 * figure_image_missile_launcher_offset(f);
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 745 + figure_image_corpse_offset(f);
    } else {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy_elephant_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, formation_get(f->formationId));
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    if (f->direction == DIR_FIGURE_ATTACK || f->actionState == FIGURE_ACTION_150_ATTACK) {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 705 + figure_image_corpse_offset(f);
    } else {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy_chariot_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 3;
    enemy_action(f, formation_get(f->formationId));
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    if (f->direction == DIR_FIGURE_ATTACK || f->actionState == FIGURE_ACTION_150_ATTACK) {
        f->graphicId = 697 + dir + 8 * (f->graphicOffset / 2);
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 745 + figure_image_corpse_offset(f);
    } else {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy49_fast_sword_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 2;
    enemy_action(f, m);
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    int attack_id, corpse_id, normal_id;
    if (m->enemy_type == ENEMY_0_BARBARIAN) {
        attack_id = 393;
        corpse_id = 441;
        normal_id = 297;
    } else if (m->enemy_type == ENEMY_1_NUMIDIAN) {
        attack_id = 593;
        corpse_id = 641;
        normal_id = 449;
    } else if (m->enemy_type == ENEMY_4_GOTH) {
        attack_id = 545;
        corpse_id = 593;
        normal_id = 449;
    } else {
        return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = attack_id + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = attack_id + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = corpse_id + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = attack_id + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = normal_id + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy50_sword_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    if (m->enemy_type != ENEMY_2_GAUL && m->enemy_type != ENEMY_3_CELT) {
        return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 545 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 545 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 593 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 545 + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = 449 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy51_spear_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 2;
    enemy_action(f, m);
    
    int dir = get_missile_direction(f, m);
    
    f->isEnemyGraphic = 1;
    
    if (m->enemy_type != ENEMY_1_NUMIDIAN) {
        return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 593 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 593 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_151_ENEMY_INITIAL) {
        f->graphicId = 545 + dir + 8 * figure_image_missile_launcher_offset(f);
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 641 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 593 + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = 449 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy52_mounted_archer_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 3;
    enemy_action(f, m);
    
    int dir = get_missile_direction(f, m);
    
    f->isEnemyGraphic = 1;
    
    if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    } else if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        f->graphicId = 601 + dir;
    } else if (f->actionState == FIGURE_ACTION_151_ENEMY_INITIAL) {
        f->graphicId = 697 + dir + 8 * figure_image_missile_launcher_offset(f);
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 745 + figure_image_corpse_offset(f);
    } else {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy53_axe_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_direction(f);
    
    f->isEnemyGraphic = 1;
    
    if (m->enemy_type != ENEMY_2_GAUL) {
        return;
    }
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 697 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 697 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 745 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 697 + dir + 8 * (f->graphicOffset / 2);
    } else {
        f->graphicId = 601 + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy_gladiator_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Any;
    f->useCrossCountry = 0;
    figure_image_increase_offset(f, 12);
    if (scenario_gladiator_revolt_is_finished()) {
        // end of gladiator revolt: kill gladiators
        if (f->actionState != FIGURE_ACTION_149_CORPSE) {
            f->actionState = FIGURE_ACTION_149_CORPSE;
            f->waitTicks = 0;
            f->direction = 0;
        }
    }
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            figure_image_increase_offset(f, 16);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_158_NATIVE_CREATED:
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks > 10 + (f->id & 3)) {
                f->waitTicks = 0;
                f->actionState = FIGURE_ACTION_159_NATIVE_ATTACKING;
                int x_tile, y_tile;
                int building_id = formation_rioter_get_target_building(&x_tile, &y_tile);
                if (building_id) {
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                    f->destinationBuildingId = building_id;
                    figure_route_remove(f);
                } else {
                    f->state = FigureState_Dead;
                }
            }
            break;
        case FIGURE_ACTION_159_NATIVE_ATTACKING:
            Data_CityInfo.numAttackingNativesInCity = 10;
            f->terrainUsage = FigureTerrainUsage_Enemy;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_158_NATIVE_CREATED;
            }
            break;
    }
    int dir;
    if (f->actionState == FIGURE_ACTION_150_ATTACK || f->direction == DIR_FIGURE_ATTACK) {
        dir = f->attackDirection;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    dir = figure_image_normalize_direction(dir);

    if (f->actionState == FIGURE_ACTION_150_ATTACK || f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = image_group(GROUP_FIGURE_GLADIATOR) + dir + 104 + 8 * (f->graphicOffset / 2);
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_GLADIATOR) + 96 + figure_image_corpse_offset(f);
    } else {
        f->graphicId = image_group(GROUP_FIGURE_GLADIATOR) + dir + 8 * f->graphicOffset;
    }
}

void figure_enemy_caesar_legionary_action(figure *f)
{
    formation *m = formation_get(f->formationId);
    Data_CityInfo.numImperialSoldiersInCity++;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    f->speedMultiplier = 1;
    enemy_action(f, m);
    
    int dir = get_direction(f);
    
    if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + dir +
            8 * ((f->attackGraphicOffset - 12) / 2);
    }
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            if (f->attackGraphicOffset >= 12) {
                f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + dir +
                    8 * ((f->attackGraphicOffset - 12) / 2);
            } else {
                f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + dir;
            }
            break;
        case FIGURE_ACTION_149_CORPSE:
            f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) +
                figure_image_corpse_offset(f) + 152;
            break;
        case FIGURE_ACTION_84_SOLDIER_AT_STANDARD:
            if (m->is_halted && m->layout == FORMATION_COLUMN && m->missile_attack_timeout) {
                f->graphicId = image_group(GROUP_BUILDING_FORT_LEGIONARY) + dir + 144;
            } else {
                f->graphicId = image_group(GROUP_BUILDING_FORT_LEGIONARY) + dir;
            }
            break;
        default:
            f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + 48 + dir + 8 * f->graphicOffset;
            break;
    }
}
