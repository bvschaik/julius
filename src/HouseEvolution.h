#ifndef HOUSEEVOLUTION_H
#define HOUSEEVOLUTION_H

void HouseEvolution_Tick_evolveAndConsumeResources();

void HouseEvolution_Tick_decayCultureService();

void HouseEvolution_Tick_calculateCultureServiceAggregates();

void HouseEvolution_determineEvolveText(int buildingId);

#endif
