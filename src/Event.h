#ifndef EVENT_H
#define EVENT_H

enum {
	RequestState_Normal = 0,
	RequestState_Overdue = 1,
	RequestState_Dispatched = 2,
	RequestState_DispatchedLate = 3,
	RequestState_Ignored = 4,
	RequestState_Received = 5,
};

void Event_initInvasions();
void Event_handleInvasions();
int Event_startInvasionLocalUprisingFromMars();
void Event_startInvasionFromCheat();
int Event_existsUpcomingInvasion();

void Event_Caesar_update();

void Event_initDistantBattleCity();
void Event_calculateDistantBattleRomanTravelTime();
void Event_calculateDistantBattleEnemyTravelTime();
void Event_handleDistantBattle();

void Event_initRequests();
void Event_handleRequests();
void Event_dispatchRequest(int id);

void Event_initDemandChanges();
void Event_handleDemandChanges();

void Event_initPriceChanges();
void Event_handlePricesChanges();

void Event_handleRandomEvents();

#endif
