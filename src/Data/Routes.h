#ifndef DATA_ROUTES_H
#define DATA_ROUTES_H

#define MAX_ROUTEPATH_LENGTH 500
#define MAX_ROUTES 600

extern struct Data_Routes {
	short walkerIds[MAX_ROUTES];
	unsigned char directionPaths[MAX_ROUTES][MAX_ROUTEPATH_LENGTH];
	
	// debug
	int totalRoutesCalculated;
	int enemyRoutesCalculated;
} Data_Routes;

#endif
