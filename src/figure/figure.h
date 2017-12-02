#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H

#include "core/buffer.h"
#include "core/direction.h"
#include "figure/action.h"
#include "figure/type.h"

#define MAX_FIGURES 1000

typedef struct {
    int id;

    unsigned char alternativeLocationIndex;
    unsigned char graphicOffset;
    unsigned char isEnemyGraphic;
    unsigned char flotsamVisible;
    short graphicId; // 04
    short cartGraphicId;
    short nextFigureIdOnSameTile; // 08
    unsigned char type;
    unsigned char resourceId; //0b
    unsigned char useCrossCountry;
    unsigned char isFriendly;
    unsigned char state;
    unsigned char ciid; // 0f
    unsigned char actionStateBeforeAttack; // 10
    signed char direction;
    signed char previousTileDirection; // 12
    signed char attackDirection; // 13
    unsigned char x;
    unsigned char y;
    unsigned char previousTileX;
    unsigned char previousTileY;
    unsigned char missileDamage;
    unsigned char damage; //19
    short gridOffset; // 1a
    unsigned char destinationX; // 1c
    unsigned char destinationY;
    short destinationGridOffsetSoldier;
    unsigned char sourceX; // 20
    unsigned char sourceY;
    signed char formationPositionX;
    signed char formationPositionY;
    short __unused_24;
    short waitTicks;
    unsigned char actionState;
    unsigned char progressOnTile; // 29
    short routingPathId;
    short routingPathCurrentTile;
    short routingPathLength;
    unsigned char inBuildingWaitTicks; // 30
    unsigned char isOnRoad;
    short maxRoamLength;
    short roamLength;
    unsigned char roamChooseDestination;
    unsigned char roamRandomCounter;
    signed char roamTurnDirection;
    signed char roamTicksUntilNextTurn;
    short crossCountryX; // 3a - position = 15 * x + offset on tile
    short crossCountryY; // 3c - position = 15 * y + offset on tile
    short ccDestinationX; // 3e
    short ccDestinationY; // 40
    short ccDeltaX; // 42
    short ccDeltaY; // 44
    short ccDeltaXY; // 46
    unsigned char ccDirection; // 48: 1 = x, 2 = y
    unsigned char speedMultiplier;
    short buildingId;
    short immigrantBuildingId;
    short destinationBuildingId;
    short formationId; //50
    unsigned char indexInFormation; //52
    unsigned char formationAtRest; //53
    unsigned char migrantNumPeople;
    unsigned char isGhost; // 55
    unsigned char minMaxSeen;
    unsigned char __unused_57;
    short inFrontFigureId;
    unsigned char attackGraphicOffset;
    unsigned char waitTicksMissile;
    signed char xOffsetCart; // 5c
    signed char yOffsetCart; // 5d
    unsigned char empireCityId; // 5e
    unsigned char traderAmountBought;
    short name; // 60
    unsigned char terrainUsage;
    unsigned char loadsSoldOrCarrying;
    unsigned char isBoat; // 64
    unsigned char heightAdjustedTicks; // 65
    unsigned char currentHeight;
    unsigned char targetHeight;
    unsigned char collectingItemId; // NOT a resource ID for cartpushers! IS a resource ID for warehousemen
    unsigned char tradeShipFailedDockAttempts;
    unsigned char phraseSequenceExact;
    signed char phraseId;
    unsigned char phraseSequenceCity;
    unsigned char traderId;
    unsigned char waitTicksNextTarget;
    unsigned char __unused_6f;
    short targetFigureId; // 70
    short targetedByFigureId; // 72
    unsigned short createdSequence;
    unsigned short targetFigureCreatedSequence;
    unsigned char numPreviousFiguresOnSameTile;
    unsigned char numAttackers;
    short attackerId1;
    short attackerId2;
    short opponentId; // 7e
} figure;

figure *figure_get(int id);

/**
 * Creates a figure
 * @param type Figure type
 * @param x X position
 * @param y Y position
 * @param dir Direction the figure faces
 * @return Always a figure. If figure->id is zero, it is an invalid one.
 */
figure *figure_create(figure_type type, int x, int y, direction dir);

int figure_is_dead(figure *f);

void figure_init_scenario();

void figure_save_state(buffer *list, buffer *seq);

void figure_load_state(buffer *list, buffer *seq);

#endif // FIGURE_FIGURE_H
