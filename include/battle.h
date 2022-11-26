#ifndef BATTLE_H
#define BATTLE_H

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define TEAM_NONE -1
#define TEAM_ENGLAND 0
#define TEAM_CYMRU 1
#define TEAM_SCOTLAND 2

#define TYPE_LAND 0
#define TYPE_WATER 1
#define TYPE_AIR 2

#define BUFF_NONE -1
#define BUFF_SIGHT 0
#define BUFF_STRENGTH 1
#define BUFF_SPEED 2

#define MAP_W 32
#define MAP_H 32

#define MAX_UNITS 12

#define CHECK_SPAWNS_CONFLICTS true

/**
 * Data form for a Tile type
 */
struct Tile
{
    int team;     // one of the defined teams above or -1 for generic
    int type;     // the type of tile (land, water, or air)
    int buffType; // the type of buff applied to home team and debuff applied to home team
    int buffStrength;
    char *name;
};

/**
 * Data form for a Unit type
 */
struct Unit
{
    char *name;
    int team;
    int type;
    int stats[3]; // stats
    bool isSignatureUnit;
    int maxHealth;
};

/**
 * Map Unit (loaded version of Unit)
 */
struct MUnit
{
    int type;
    int health;
    int movement;
    bool hasAttackedThisTurn;
    bool isVisibleThisTurn;
    int x, y;
};

struct UnitSpawn{
    int type;
    int x;
    int y;
};

/**
 * Every unit's static data
 */
extern const struct Unit allUnits[12];

/**
 * Every tile's static data
 */
extern const struct Tile allTiles[12];

/**
 * The current map's tiles
 */
extern const int mapTiles[MAP_W * MAP_H];

/**
 * The current map's visible tiles
 */
extern bool visibleMapTiles[MAP_W * MAP_H];

/**
 * The currently loaded units
 */
extern struct MUnit loadedUnits[MAX_UNITS * 3];

/**
 * Begins the turn for a particular team, setting all loaded units' values
 * @param team the team to start for
 */
void startTurnFor(int team);

/**
 * Moves a unit to a particular tile
 * @param unitID the unit to move
 * @param x
 * @param y
 * @return bool true if the move was successful
 */
bool moveUnitTo(int unitID, int x, int y);

/**
 * Has a unit attack another unit
 * @param unitID the unit to move
 * @param targetUnitID the unit to attack
 * @return bool true if attack was successful
 */
bool attackUnit(int unitID, int targetUnitID);

void loadUnits(struct UnitSpawn* spawns);

#endif