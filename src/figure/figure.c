#include "figure/figure.h"

#include <string.h>

struct Data_Figure *figure_get(int id)
{
    return &Data_Figures[id];
}

void figure_init_scenario()
{
    for (int i = 0; i < MAX_FIGURES; i++) {
        memset(&Data_Figures[i], 0, sizeof(figure));
        Data_Figures[i].id = i;
    }
}

static void figure_save(buffer *buf, struct Data_Figure *f)
{
    buffer_write_u8(buf, f->alternativeLocationIndex);
    buffer_write_u8(buf, f->graphicOffset);
    buffer_write_u8(buf, f->isEnemyGraphic);
    buffer_write_u8(buf, f->flotsamVisible);
    buffer_write_i16(buf, f->graphicId);
    buffer_write_i16(buf, f->cartGraphicId);
    buffer_write_i16(buf, f->nextFigureIdOnSameTile);
    buffer_write_u8(buf, f->type);
    buffer_write_u8(buf, f->resourceId);
    buffer_write_u8(buf, f->useCrossCountry);
    buffer_write_u8(buf, f->isFriendly);
    buffer_write_u8(buf, f->state);
    buffer_write_u8(buf, f->ciid);
    buffer_write_u8(buf, f->actionStateBeforeAttack);
    buffer_write_i8(buf, f->direction);
    buffer_write_i8(buf, f->previousTileDirection);
    buffer_write_i8(buf, f->attackDirection);
    buffer_write_u8(buf, f->x);
    buffer_write_u8(buf, f->y);
    buffer_write_u8(buf, f->previousTileX);
    buffer_write_u8(buf, f->previousTileY);
    buffer_write_u8(buf, f->missileDamage);
    buffer_write_u8(buf, f->damage);
    buffer_write_i16(buf, f->gridOffset);
    buffer_write_u8(buf, f->destinationX);
    buffer_write_u8(buf, f->destinationY);
    buffer_write_i16(buf, f->destinationGridOffsetSoldier);
    buffer_write_u8(buf, f->sourceX);
    buffer_write_u8(buf, f->sourceY);
    buffer_write_i8(buf, f->formationPositionX);
    buffer_write_i8(buf, f->formationPositionY);
    buffer_write_i16(buf, f->__unused_24);
    buffer_write_i16(buf, f->waitTicks);
    buffer_write_u8(buf, f->actionState);
    buffer_write_u8(buf, f->progressOnTile);
    buffer_write_i16(buf, f->routingPathId);
    buffer_write_i16(buf, f->routingPathCurrentTile);
    buffer_write_i16(buf, f->routingPathLength);
    buffer_write_u8(buf, f->inBuildingWaitTicks);
    buffer_write_u8(buf, f->isOnRoad);
    buffer_write_i16(buf, f->maxRoamLength);
    buffer_write_i16(buf, f->roamLength);
    buffer_write_u8(buf, f->roamChooseDestination);
    buffer_write_u8(buf, f->roamRandomCounter);
    buffer_write_i8(buf, f->roamTurnDirection);
    buffer_write_i8(buf, f->roamTicksUntilNextTurn);
    buffer_write_i16(buf, f->crossCountryX);
    buffer_write_i16(buf, f->crossCountryY);
    buffer_write_i16(buf, f->ccDestinationX);
    buffer_write_i16(buf, f->ccDestinationY);
    buffer_write_i16(buf, f->ccDeltaX);
    buffer_write_i16(buf, f->ccDeltaY);
    buffer_write_i16(buf, f->ccDeltaXY);
    buffer_write_u8(buf, f->ccDirection);
    buffer_write_u8(buf, f->speedMultiplier);
    buffer_write_i16(buf, f->buildingId);
    buffer_write_i16(buf, f->immigrantBuildingId);
    buffer_write_i16(buf, f->destinationBuildingId);
    buffer_write_i16(buf, f->formationId);
    buffer_write_u8(buf, f->indexInFormation);
    buffer_write_u8(buf, f->formationAtRest);
    buffer_write_u8(buf, f->migrantNumPeople);
    buffer_write_u8(buf, f->isGhost);
    buffer_write_u8(buf, f->minMaxSeen);
    buffer_write_u8(buf, f->__unused_57);
    buffer_write_i16(buf, f->inFrontFigureId);
    buffer_write_u8(buf, f->attackGraphicOffset);
    buffer_write_u8(buf, f->waitTicksMissile);
    buffer_write_i8(buf, f->xOffsetCart);
    buffer_write_i8(buf, f->yOffsetCart);
    buffer_write_u8(buf, f->empireCityId);
    buffer_write_u8(buf, f->traderAmountBought);
    buffer_write_i16(buf, f->name);
    buffer_write_u8(buf, f->terrainUsage);
    buffer_write_u8(buf, f->loadsSoldOrCarrying);
    buffer_write_u8(buf, f->isBoat);
    buffer_write_u8(buf, f->heightAdjustedTicks);
    buffer_write_u8(buf, f->currentHeight);
    buffer_write_u8(buf, f->targetHeight);
    buffer_write_u8(buf, f->collectingItemId);
    buffer_write_u8(buf, f->tradeShipFailedDockAttempts);
    buffer_write_u8(buf, f->phraseSequenceExact);
    buffer_write_i8(buf, f->phraseId);
    buffer_write_u8(buf, f->phraseSequenceCity);
    buffer_write_u8(buf, f->traderId);
    buffer_write_u8(buf, f->waitTicksNextTarget);
    buffer_write_u8(buf, f->__unused_6f);
    buffer_write_i16(buf, f->targetFigureId);
    buffer_write_i16(buf, f->targetedByFigureId);
    buffer_write_u16(buf, f->createdSequence);
    buffer_write_u16(buf, f->targetFigureCreatedSequence);
    buffer_write_u8(buf, f->numPreviousFiguresOnSameTile);
    buffer_write_u8(buf, f->numAttackers);
    buffer_write_i16(buf, f->attackerId1);
    buffer_write_i16(buf, f->attackerId2);
    buffer_write_i16(buf, f->opponentId);
}

static void figure_load(buffer *buf, struct Data_Figure *f)
{
    f->alternativeLocationIndex = buffer_read_u8(buf);
    f->graphicOffset = buffer_read_u8(buf);
    f->isEnemyGraphic = buffer_read_u8(buf);
    f->flotsamVisible = buffer_read_u8(buf);
    f->graphicId = buffer_read_i16(buf);
    f->cartGraphicId = buffer_read_i16(buf);
    f->nextFigureIdOnSameTile = buffer_read_i16(buf);
    f->type = buffer_read_u8(buf);
    f->resourceId = buffer_read_u8(buf);
    f->useCrossCountry = buffer_read_u8(buf);
    f->isFriendly = buffer_read_u8(buf);
    f->state = buffer_read_u8(buf);
    f->ciid = buffer_read_u8(buf);
    f->actionStateBeforeAttack = buffer_read_u8(buf);
    f->direction = buffer_read_i8(buf);
    f->previousTileDirection = buffer_read_i8(buf);
    f->attackDirection = buffer_read_i8(buf);
    f->x = buffer_read_u8(buf);
    f->y = buffer_read_u8(buf);
    f->previousTileX = buffer_read_u8(buf);
    f->previousTileY = buffer_read_u8(buf);
    f->missileDamage = buffer_read_u8(buf);
    f->damage = buffer_read_u8(buf);
    f->gridOffset = buffer_read_i16(buf);
    f->destinationX = buffer_read_u8(buf);
    f->destinationY = buffer_read_u8(buf);
    f->destinationGridOffsetSoldier = buffer_read_i16(buf);
    f->sourceX = buffer_read_u8(buf);
    f->sourceY = buffer_read_u8(buf);
    f->formationPositionX = buffer_read_i8(buf);
    f->formationPositionY = buffer_read_i8(buf);
    f->__unused_24 = buffer_read_i16(buf);
    f->waitTicks = buffer_read_i16(buf);
    f->actionState = buffer_read_u8(buf);
    f->progressOnTile = buffer_read_u8(buf);
    f->routingPathId = buffer_read_i16(buf);
    f->routingPathCurrentTile = buffer_read_i16(buf);
    f->routingPathLength = buffer_read_i16(buf);
    f->inBuildingWaitTicks = buffer_read_u8(buf);
    f->isOnRoad = buffer_read_u8(buf);
    f->maxRoamLength = buffer_read_i16(buf);
    f->roamLength = buffer_read_i16(buf);
    f->roamChooseDestination = buffer_read_u8(buf);
    f->roamRandomCounter = buffer_read_u8(buf);
    f->roamTurnDirection = buffer_read_i8(buf);
    f->roamTicksUntilNextTurn = buffer_read_i8(buf);
    f->crossCountryX = buffer_read_i16(buf);
    f->crossCountryY = buffer_read_i16(buf);
    f->ccDestinationX = buffer_read_i16(buf);
    f->ccDestinationY = buffer_read_i16(buf);
    f->ccDeltaX = buffer_read_i16(buf);
    f->ccDeltaY = buffer_read_i16(buf);
    f->ccDeltaXY = buffer_read_i16(buf);
    f->ccDirection = buffer_read_u8(buf);
    f->speedMultiplier = buffer_read_u8(buf);
    f->buildingId = buffer_read_i16(buf);
    f->immigrantBuildingId = buffer_read_i16(buf);
    f->destinationBuildingId = buffer_read_i16(buf);
    f->formationId = buffer_read_i16(buf);
    f->indexInFormation = buffer_read_u8(buf);
    f->formationAtRest = buffer_read_u8(buf);
    f->migrantNumPeople = buffer_read_u8(buf);
    f->isGhost = buffer_read_u8(buf);
    f->minMaxSeen = buffer_read_u8(buf);
    f->__unused_57 = buffer_read_u8(buf);
    f->inFrontFigureId = buffer_read_i16(buf);
    f->attackGraphicOffset = buffer_read_u8(buf);
    f->waitTicksMissile = buffer_read_u8(buf);
    f->xOffsetCart = buffer_read_i8(buf);
    f->yOffsetCart = buffer_read_i8(buf);
    f->empireCityId = buffer_read_u8(buf);
    f->traderAmountBought = buffer_read_u8(buf);
    f->name = buffer_read_i16(buf);
    f->terrainUsage = buffer_read_u8(buf);
    f->loadsSoldOrCarrying = buffer_read_u8(buf);
    f->isBoat = buffer_read_u8(buf);
    f->heightAdjustedTicks = buffer_read_u8(buf);
    f->currentHeight = buffer_read_u8(buf);
    f->targetHeight = buffer_read_u8(buf);
    f->collectingItemId = buffer_read_u8(buf);
    f->tradeShipFailedDockAttempts = buffer_read_u8(buf);
    f->phraseSequenceExact = buffer_read_u8(buf);
    f->phraseId = buffer_read_i8(buf);
    f->phraseSequenceCity = buffer_read_u8(buf);
    f->traderId = buffer_read_u8(buf);
    f->waitTicksNextTarget = buffer_read_u8(buf);
    f->__unused_6f = buffer_read_u8(buf);
    f->targetFigureId = buffer_read_i16(buf);
    f->targetedByFigureId = buffer_read_i16(buf);
    f->createdSequence = buffer_read_u16(buf);
    f->targetFigureCreatedSequence = buffer_read_u16(buf);
    f->numPreviousFiguresOnSameTile = buffer_read_u8(buf);
    f->numAttackers = buffer_read_u8(buf);
    f->attackerId1 = buffer_read_i16(buf);
    f->attackerId2 = buffer_read_i16(buf);
    f->opponentId = buffer_read_i16(buf);
}

void figure_save_state(buffer *list, buffer *seq)
{
    buffer_write_i32(seq, Data_Figure_Extra.createdSequence);

    for (int i = 0; i < MAX_FIGURES; i++) {
        figure_save(list, &Data_Figures[i]);
    }
}

void figure_load_state(buffer *list, buffer *seq)
{
    Data_Figure_Extra.createdSequence = buffer_read_i32(seq);

    for (int i = 0; i < MAX_FIGURES; i++) {
        figure_load(list, &Data_Figures[i]);
        Data_Figures[i].id = i;
    }
}
