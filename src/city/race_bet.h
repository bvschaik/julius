#ifndef CITY_RACE_BET_H
#define CITY_RACE_BET_H

typedef enum {
    NO_BET,
    BLUE_HORSE,
    RED_HORSE,
    WHITE_HORSE,
    GREEN_HORSE
} bet_horse;

int has_bet_in_progress(void);
void race_result_process(void);

#endif //CITY_RACE_BET_H
