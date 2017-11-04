#include "loki/loki.h"
#include "figure/name.hpp"

#include "mocks/buffer.h"
#include "mocks/random.h"

CREATE_BUFFER_MOCKS
CREATE_RANDOM_MOCKS

INIT_MOCKS(
    INIT_BUFFER_MOCKS
    INIT_RANDOM_MOCKS
)

buffer buf;

void test_figure_name_init()
{
    when_random_byte()->then_return = 100;
    figure_name_init();

    verify_random_generate_next_times(21);
    verify_random_byte_times(21);

    figure_name_save_state(&buf);
    verify_buffer_write_i32(&buf, 100 & 0xf);
}

void test_figure_name_get_increases_count()
{
    figure_name_init();

    int n1 = figure_name_get(FIGURE_ACTOR, (enemy_type)0);
    int n2 = figure_name_get(FIGURE_ACTOR, (enemy_type)0);

    assert_eq(n1 + 1, n2);
}

void test_figure_name_get_wraps()
{
    figure_name_init();

    int n0 = figure_name_get(FIGURE_ACTOR, (enemy_type)0);
    for (int i = 0; i < 31; i++)
    {
        figure_name_get(FIGURE_ACTOR, (enemy_type)i);
    }
    int n32 = figure_name_get(FIGURE_ACTOR, (enemy_type)0);

    assert_eq(n0, n32);
}

static void check(figure_type type, int expected)
{
    int result = figure_name_get(type, (enemy_type)0);
    assert_eq(expected, result);
}

void test_figure_name_get_types()
{
    figure_name_init();

    check(FIGURE_TAX_COLLECTOR, 132);
    check(FIGURE_ENGINEER, 165);
    check(FIGURE_PREFECT, 198);
    check(FIGURE_TOWER_SENTRY, 199);
    check(FIGURE_ACTOR, 330);
    check(FIGURE_GLADIATOR, 363);
    check(FIGURE_LION_TAMER, 396);
    check(FIGURE_CHARIOTEER, 413);
    check(FIGURE_TRADE_CARAVAN, 562);
    check(FIGURE_TRADE_CARAVAN_DONKEY, 563);
    check(FIGURE_TRADE_SHIP, 579);
    check(FIGURE_FISHING_BOAT, 580);
    check(FIGURE_MARKET_TRADER, 99);
    check(FIGURE_MARKET_BUYER, 100);
    check(FIGURE_BATHHOUSE_WORKER, 101);
    check(FIGURE_SCHOOL_CHILD, 1);
    check(FIGURE_DELIVERY_BOY, 2);
    check(FIGURE_BARBER, 3);
    check(FIGURE_WORKER, 4);
    check(FIGURE_PRIEST, 66);
    check(FIGURE_TEACHER, 67);
    check(FIGURE_MISSIONARY, 68);
    check(FIGURE_LIBRARIAN, 69);
    check(FIGURE_DOCTOR, 70);
    check(FIGURE_SURGEON, 71);
    check(FIGURE_PATRICIAN, 72);
    check(FIGURE_FORT_JAVELIN, 231);
    check(FIGURE_ENEMY55_JAVELIN, 232);
    check(FIGURE_FORT_MOUNTED, 264);
    check(FIGURE_ENEMY56_MOUNTED, 265);
    check(FIGURE_FORT_LEGIONARY, 297);
    check(FIGURE_ENEMY_CAESAR_LEGIONARY, 298);
    check(FIGURE_INDIGENOUS_NATIVE, 430);
    check(FIGURE_NATIVE_TRADER, 431);

    check(FIGURE_EXPLOSION, 0);
    check(FIGURE_FORT_STANDARD, 0);
    check(FIGURE_FISH_GULLS, 0);
    check(FIGURE_CREATURE, 0);
    check(FIGURE_HIPPODROME_HORSES, 0);
}

static void check_enemy(figure_type type, enemy_type enemy, int expected)
{
    int result = figure_name_get(type, enemy);
    assert_eq(expected, result);
}

void test_figure_name_get_enemy()
{
    figure_name_init();

    check_enemy(FIGURE_ENEMY43_SPEAR, ENEMY_8_GREEK, 463);
    check_enemy(FIGURE_ENEMY44_SWORD, ENEMY_9_EGYPTIAN, 496);
    check_enemy(FIGURE_ENEMY45_SWORD, ENEMY_1_NUMIDIAN, 529);
    check_enemy(FIGURE_ENEMY46_CAMEL, ENEMY_5_PERGAMUM, 530);
    check_enemy(FIGURE_ENEMY47_ELEPHANT, ENEMY_10_CARTHAGINIAN, 531);
    check_enemy(FIGURE_ENEMY48_CHARIOT, ENEMY_7_ETRUSCAN, 198);
    check_enemy(FIGURE_ENEMY49_FAST_SWORD, ENEMY_2_GAUL, 430);
    check_enemy(FIGURE_ENEMY50_SWORD, ENEMY_3_CELT, 431);
    check_enemy(FIGURE_ENEMY51_SPEAR, ENEMY_4_GOTH, 432);
    check_enemy(FIGURE_ENEMY52_MOUNTED_ARCHER, ENEMY_6_SELEUCID, 433);
    check_enemy(FIGURE_ENEMY53_AXE, ENEMY_0_BARBARIAN, 434);
}

void test_figure_name_save_state()
{
    figure_name_save_state(&buf);

    verify_buffer_write_i32_times(21);
}

void test_figure_name_load_state()
{
    figure_name_load_state(&buf);

    verify_buffer_read_i32_times(21);
}

RUN_TESTS(figure.name,
          ADD_TEST(test_figure_name_init)
          ADD_TEST(test_figure_name_get_increases_count)
          ADD_TEST(test_figure_name_get_wraps)
          ADD_TEST(test_figure_name_get_types)
          ADD_TEST(test_figure_name_get_enemy)
          ADD_TEST(test_figure_name_save_state)
          ADD_TEST(test_figure_name_load_state)
         )
