#ifndef GAME_DIFFICULTY_H
#define GAME_DIFFICULTY_H

int difficulty_starting_favor(void);

int difficulty_sentiment(void);

int difficulty_base_tax_rate(void);

int difficulty_adjust_money(int money);

int difficulty_adjust_enemies(int enemies);

int difficulty_adjust_wolf_attack(int attack);

int difficulty_adjust_levies(int amount);

int difficulty_adjust_soldier_food_consumption(int amount);

#endif // GAME_DIFFICULTY_H
