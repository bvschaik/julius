#ifndef DATA_ROUTES_H
#define DATA_ROUTES_H

#define MAX_ROUTEPATH_LENGTH 500
#define MAX_ROUTES 600

extern struct _Data_Routes {
	short figureIds[MAX_ROUTES];
	unsigned char directionPaths[MAX_ROUTES][MAX_ROUTEPATH_LENGTH];
	
	// debug
	int totalRoutesCalculated;
	int enemyRoutesCalculated;
	int unknown1RoutesCalculated;
	int unknown2RoutesCalculated;
} Data_Routes;

#endif
