#include "difficulty.h"

#include "core/calc.h"
#include "game/settings.h"

static const struct {
    int money;
    int enemies;
    int starting_favor;
    int sentiment;
    int levies;
    int food_per_soldier;
} data[] = {
    {300, 40, 70, 80, 0, -3}, // very easy
    {200, 60, 60, 70, 0, -3}, // easy
    {150, 80, 50, 60, 50, -2}, // normal
    {100, 100, 50, 50, 100, 0}, // hard
    {75, 120, 40, 40, 100, 1} // very hard
};

int difficulty_starting_favor(void)
{
    return data[setting_difficulty()].starting_favor;
}

int difficulty_sentiment(void)
{
    return data[setting_difficulty()].sentiment;
}

int difficulty_adjust_money(int money)
{
    return calc_adjust_with_percentage(money, data[setting_difficulty()].money);
}

int difficulty_adjust_enemies(int enemies)
{
    return calc_adjust_with_percentage(enemies, data[setting_difficulty()].enemies);
}

int difficulty_adjust_levies(int amount)
{
    return calc_adjust_with_percentage(amount, data[setting_difficulty()].levies);
}

int difficulty_adjust_soldier_food_consumption(int amount)
{
    return amount + data[setting_difficulty()].food_per_soldier;
}

int difficulty_adjust_wolf_attack(int attack)
{
    switch (setting_difficulty()) {
    case DIFFICULTY_VERY_EASY: return 2;
    case DIFFICULTY_EASY: return 4;
    case DIFFICULTY_NORMAL: return 6;
    default: return attack;
    }
}
