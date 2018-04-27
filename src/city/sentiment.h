#ifndef CITY_SENTIMENT_H
#define CITY_SENTIMENT_H

int city_sentiment();

int city_sentiment_low_mood_cause();

void city_sentiment_change_happiness(int amount);

void city_sentiment_set_max_happiness(int max);

void city_sentiment_reset_protesters_criminals();

void city_sentiment_add_protester();
void city_sentiment_add_criminal();

int city_sentiment_protesters();
int city_sentiment_criminals();

void city_sentiment_update();

#endif // CITY_SENTIMENT_H
