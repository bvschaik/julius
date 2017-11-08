#ifndef UNDO_H
#define UNDO_H

struct Undo
{
    static int recordBeforeBuild();
    static void restoreTerrainGraphics();
    static void restoreBuildings();
    static void addBuildingToList(int buildingId);
    static int isBuildingInList(int buildingId);
    static void recordBuild(int cost);
    static void perform();
    static void updateAvailable();
};

#endif
