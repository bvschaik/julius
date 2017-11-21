#ifndef EVENT_H
#define EVENT_H

void Event_initInvasions();
void Event_handleInvasions();
int Event_startInvasionLocalUprisingFromMars();
void Event_startInvasionFromCheat();
int Event_existsUpcomingInvasion();

void Event_Caesar_update();

void Event_calculateDistantBattleRomanTravelTime();
void Event_calculateDistantBattleEnemyTravelTime();
void Event_handleDistantBattle();

void Event_handleRandomEvents();

void Event_handleEmperorChange();

#endif
