#include "figure/name.h"

#include "core/random.h"

static struct {
    int32_t citizen_male;
    int32_t patrician;
    int32_t citizen_female;
    int32_t tax_collector;
    int32_t engineer;
    int32_t prefect;
    int32_t javelin_thrower;
    int32_t cavalry;
    int32_t legionary;
    int32_t actor;
    int32_t gladiator;
    int32_t lion_tamer;
    int32_t charioteer;
    int32_t barbarian;
    int32_t enemy_greek;
    int32_t enemy_egyptian;
    int32_t enemy_arabian;
    int32_t trader;
    int32_t ship;
    int32_t warship;
    int32_t enemy_warship;
} data;

static int32_t init_name(void)
{
    random_generate_next();
    return random_byte() & 0xf;
}

void figure_name_init(void)
{
    data.citizen_male = init_name();
    data.patrician = init_name();
    data.citizen_female = init_name();
    data.tax_collector = init_name();
    data.engineer = init_name();
    data.prefect = init_name();
    data.javelin_thrower = init_name();
    data.cavalry = init_name();
    data.legionary = init_name();
    data.actor = init_name();
    data.gladiator = init_name();
    data.lion_tamer = init_name();
    data.charioteer = init_name();
    data.barbarian = init_name();
    data.enemy_greek = init_name();
    data.enemy_egyptian = init_name();
    data.enemy_arabian = init_name();
    data.trader = init_name();
    data.ship = init_name();
    data.warship = init_name();
    data.enemy_warship = init_name();
}

static int get_next_name(int32_t *field, int offset, int max)
{
    int name = offset + *field;
    *field = *field + 1;
    if (*field >= max) {
        *field = 0;
    }
    return name;
}

int figure_name_get(figure_type type, enemy_type_t enemy)
{
    switch (type) {
    case FIGURE_TAX_COLLECTOR:
        return get_next_name(&data.tax_collector, 132, 32);

    case FIGURE_ENGINEER:
    case FIGURE_WORK_CAMP_ARCHITECT:
        return get_next_name(&data.engineer, 165, 32);

    case FIGURE_PREFECT:
    case FIGURE_TOWER_SENTRY:
    case FIGURE_WATCHMAN:
        return get_next_name(&data.prefect, 198, 32);

    case FIGURE_ACTOR:
        return get_next_name(&data.actor, 330, 32);

    case FIGURE_GLADIATOR:
        return get_next_name(&data.gladiator, 363, 32);

    case FIGURE_LION_TAMER:
        return get_next_name(&data.lion_tamer, 396, 16);

    case FIGURE_CHARIOTEER:
        return get_next_name(&data.charioteer, 413, 16);

    case FIGURE_TRADE_CARAVAN:
    case FIGURE_TRADE_CARAVAN_DONKEY:
        return get_next_name(&data.trader, 562, 16);

    case FIGURE_TRADE_SHIP:
    case FIGURE_FISHING_BOAT:
        return get_next_name(&data.ship, 579, 16);

    case FIGURE_MARKET_TRADER:
    case FIGURE_MARKET_SUPPLIER:
    case FIGURE_BATHHOUSE_WORKER:
    case FIGURE_BARKEEP_SUPPLIER:
    case FIGURE_BARKEEP:
        return get_next_name(&data.citizen_female, 99, 32);

    case FIGURE_SCHOOL_CHILD:
    case FIGURE_DELIVERY_BOY:
    case FIGURE_BARBER:
    case FIGURE_WORKER:
    case FIGURE_MESS_HALL_COLLECTOR:
    case FIGURE_CARAVANSERAI_SUPPLIER:
    case FIGURE_LIGHTHOUSE_SUPPLIER:
    default:
        return get_next_name(&data.citizen_male, 1, 64);

    case FIGURE_PRIEST:
    case FIGURE_PRIEST_SUPPLIER:
    case FIGURE_TEACHER:
    case FIGURE_MISSIONARY:
    case FIGURE_LIBRARIAN:
    case FIGURE_DOCTOR:
    case FIGURE_SURGEON:
    case FIGURE_PATRICIAN:
    case FIGURE_WORK_CAMP_WORKER:
        return get_next_name(&data.patrician, 66, 32);

    case FIGURE_FORT_JAVELIN:
    case FIGURE_ENEMY_CAESAR_JAVELIN:
        return get_next_name(&data.javelin_thrower, 231, 32);

    case FIGURE_FORT_MOUNTED:
    case FIGURE_ENEMY_CAESAR_MOUNTED:
        return get_next_name(&data.cavalry, 264, 32);

    case FIGURE_FORT_LEGIONARY:
    case FIGURE_ENEMY_CAESAR_LEGIONARY:
    case FIGURE_MESS_HALL_SUPPLIER:
        return get_next_name(&data.legionary, 297, 32);

    case FIGURE_INDIGENOUS_NATIVE:
    case FIGURE_NATIVE_TRADER:
    case FIGURE_WORK_CAMP_SLAVE:
        return get_next_name(&data.barbarian, 430, 32);

    case FIGURE_ENEMY43_SPEAR:
    case FIGURE_ENEMY44_SWORD:
    case FIGURE_ENEMY45_SWORD:
    case FIGURE_ENEMY46_CAMEL:
    case FIGURE_ENEMY47_ELEPHANT:
    case FIGURE_ENEMY48_CHARIOT:
    case FIGURE_ENEMY49_FAST_SWORD:
    case FIGURE_ENEMY50_SWORD:
    case FIGURE_ENEMY51_SPEAR:
    case FIGURE_ENEMY52_MOUNTED_ARCHER:
    case FIGURE_ENEMY53_AXE:
        switch (enemy) {
        case ENEMY_8_GREEK:
            return get_next_name(&data.enemy_greek, 463, 32);

        case ENEMY_9_EGYPTIAN:
            return get_next_name(&data.enemy_egyptian, 496, 32);

        case ENEMY_1_NUMIDIAN:
        case ENEMY_5_PERGAMUM:
        case ENEMY_10_CARTHAGINIAN:
            return get_next_name(&data.enemy_arabian, 529, 32);

        case ENEMY_7_ETRUSCAN:
            return get_next_name(&data.prefect, 198, 32);

        default:
            return get_next_name(&data.barbarian, 430, 32);
        }

    case FIGURE_EXPLOSION:
    case FIGURE_FORT_STANDARD:
    case FIGURE_FISH_GULLS:
    case FIGURE_CREATURE:
    case FIGURE_HIPPODROME_HORSES:
        return 0;
    }
}

void figure_name_save_state(buffer *buf)
{
    buffer_write_i32(buf, data.citizen_male);
    buffer_write_i32(buf, data.patrician);
    buffer_write_i32(buf, data.citizen_female);
    buffer_write_i32(buf, data.tax_collector);
    buffer_write_i32(buf, data.engineer);
    buffer_write_i32(buf, data.prefect);
    buffer_write_i32(buf, data.javelin_thrower);
    buffer_write_i32(buf, data.cavalry);
    buffer_write_i32(buf, data.legionary);
    buffer_write_i32(buf, data.actor);
    buffer_write_i32(buf, data.gladiator);
    buffer_write_i32(buf, data.lion_tamer);
    buffer_write_i32(buf, data.charioteer);
    buffer_write_i32(buf, data.barbarian);
    buffer_write_i32(buf, data.enemy_greek);
    buffer_write_i32(buf, data.enemy_egyptian);
    buffer_write_i32(buf, data.enemy_arabian);
    buffer_write_i32(buf, data.trader);
    buffer_write_i32(buf, data.ship);
    buffer_write_i32(buf, data.warship);
    buffer_write_i32(buf, data.enemy_warship);
}

void figure_name_load_state(buffer *buf)
{
    data.citizen_male = buffer_read_i32(buf);
    data.patrician = buffer_read_i32(buf);
    data.citizen_female = buffer_read_i32(buf);
    data.tax_collector = buffer_read_i32(buf);
    data.engineer = buffer_read_i32(buf);
    data.prefect = buffer_read_i32(buf);
    data.javelin_thrower = buffer_read_i32(buf);
    data.cavalry = buffer_read_i32(buf);
    data.legionary = buffer_read_i32(buf);
    data.actor = buffer_read_i32(buf);
    data.gladiator = buffer_read_i32(buf);
    data.lion_tamer = buffer_read_i32(buf);
    data.charioteer = buffer_read_i32(buf);
    data.barbarian = buffer_read_i32(buf);
    data.enemy_greek = buffer_read_i32(buf);
    data.enemy_egyptian = buffer_read_i32(buf);
    data.enemy_arabian = buffer_read_i32(buf);
    data.trader = buffer_read_i32(buf);
    data.ship = buffer_read_i32(buf);
    data.warship = buffer_read_i32(buf);
    data.enemy_warship = buffer_read_i32(buf);
}
