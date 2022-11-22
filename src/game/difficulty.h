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

int difficulty_favor_to_pause_emperor_attack(void);

int difficulty_favor_to_stop_emperor_attack(void);

int difficulty_random_event_cooldown_months(void);

int difficulty_high_salary_punishment(void);

#endif // GAME_DIFFICULTY_H
