#ifndef HOUSEPOPULATION_H
#define HOUSEPOPULATION_H

void HousePopulation_updateRomeAccess();
void HousePopulation_updateRoom();
void HousePopulation_updateMigration();
void HousePopulation_evictOvercrowded();

int HousePopulation_addPeople(int amount);
int HousePopulation_removePeople(int amount);

int HousePopulation_calculatePeoplePerType();

void HousePopulation_createHomeless(int x, int y, int numPeople);

#endif
