
#include "battle.h"

struct MUnit loadedUnits[MAX_UNITS * 3];
int currentTeam;

const struct Tile allTiles[12] = {
    {TEAM_NONE, TYPE_LAND, BUFF_NONE, 0, "Plains"},
    {TEAM_NONE, TYPE_WATER, BUFF_NONE, 0, "Sea"},
    {TEAM_NONE, TYPE_AIR, BUFF_NONE, 0, "Mountains"},

    {TEAM_ENGLAND, TYPE_LAND, BUFF_STRENGTH, 1, "Castle"},
    {TEAM_ENGLAND, TYPE_WATER, BUFF_SPEED, 2, "River"},
    {TEAM_ENGLAND, TYPE_AIR, BUFF_SIGHT, 1, "Pennine"},

    {TEAM_CYMRU, TYPE_LAND, BUFF_SPEED, 1, "Valley"},
    {TEAM_CYMRU, TYPE_WATER, BUFF_STRENGTH, 1, "Waterfall"},
    {TEAM_CYMRU, TYPE_AIR, BUFF_SIGHT, 2, "Mountain Range"},

    {TEAM_SCOTLAND, TYPE_LAND, BUFF_SPEED, 1, "Bog"},
    {TEAM_SCOTLAND, TYPE_WATER, BUFF_SIGHT, 1, "Loch"},
    {TEAM_SCOTLAND, TYPE_AIR, BUFF_STRENGTH, 2, "Highland"}};

const struct Unit allUnits[12] = {
    {"Y Ddraig Goch", TEAM_CYMRU, TYPE_AIR,
     (int[3]){8, 6, 6}, true, 6},

    {"Barbary Lion", TEAM_ENGLAND, TYPE_LAND,
     (int[3]){6, 6, 8}, true, 10},

    {"Loch Ness Monster", TEAM_SCOTLAND, TYPE_WATER,
     (int[3]){6, 8, 6}, true, 10},

    {"Longboat", TEAM_SCOTLAND, TYPE_WATER,
     (int[3]){4, 3, 4}, true, 4}, // total 11
    {"Clansmen", TEAM_SCOTLAND, TYPE_LAND,
     (int[3]){2, 3, 1}, true, 5}, // total 6
    {"Teuchtars", TEAM_SCOTLAND, TYPE_AIR,
     (int[3]){3, 2, 1}, true, 2}, // total 6

    {"Mountaineers", TEAM_ENGLAND, TYPE_AIR,
     (int[3]){3, 1, 2}, true, 2}, // total 6
    {"Privateers", TEAM_ENGLAND, TYPE_WATER,
     (int[3]){2, 2, 2}, true, 4}, // total 6
    {"Cavalry", TEAM_ENGLAND, TYPE_LAND,
     (int[3]){3, 3, 5}, true, 5}, // total 11

    {"Rangers", TEAM_CYMRU, TYPE_AIR,
     (int[3]){3, 5, 3}, true, 2}, // total 11
    {"Fishermen", TEAM_CYMRU, TYPE_WATER,
     (int[3]){1, 2, 3}, true, 4}, // total 6
    {"Spearmen", TEAM_CYMRU, TYPE_LAND,
     (int[3]){1, 3, 2}, true, 5} // total 6
};

void floodFillVisibleFrom(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H)
        return;

    if (visibleMapTiles[(y * MAP_W) + x] == true) // if the tile is already visible return
        return;

    visibleMapTiles[(y * MAP_W) + x] = true; // set this tile visible

    // recursively visit all tiles in a 4 way direction
    floodFillVisibleFrom(x + 1, y);
    floodFillVisibleFrom(x - 1, y);
    floodFillVisibleFrom(x, y + 1);
    floodFillVisibleFrom(x, y - 1);
}

void setTilevisible(int x, int y)
{
    if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H)
    {
        visibleMapTiles[(y * MAP_W) + x] = true;
    }
}

void calculateVisibleTiles(int team)
{
    for (int i = 0; i < MAP_H * MAP_W; i++)
    {
        visibleMapTiles[i] = false;
    }
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct Unit u = allUnits[loadedUnits[i].type];
        if (u.team == team)
        {
            int r = u.stats[BUFF_SIGHT];
            int x = loadedUnits[i].x;
            int y = loadedUnits[i].y;
            int d = 0;
            for (int a = 0; a <= r; a++)
            {
                for (int vx = a; vx >= 0; vx--)
                {
                    for (int vy = r - a; vy >= 0; vy--)
                    {
                        setTilevisible(x + vx, y + vy);
                        setTilevisible(x + vx, y - vy);
                        setTilevisible(x - vx, y + vy);
                        setTilevisible(x - vx, y - vy);
                    }
                }
            }

        }
    }
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct Unit u = allUnits[loadedUnits[i].type];
        if (u.team != team)
        {
            loadedUnits[i].isVisibleThisTurn = visibleMapTiles[loadedUnits[i].x + (loadedUnits[i].y * MAP_W)];
        }
    }
}

void startTurnFor(int team)
{
    calculateVisibleTiles(team);
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct Unit u = allUnits[loadedUnits[i].type];
        if (u.team == team)
        {
            loadedUnits[i].hasAttackedThisTurn = false;
            loadedUnits[i].movement = u.stats[BUFF_SPEED];
            loadedUnits[i].isVisibleThisTurn = true;
        }
        else
        {
            loadedUnits[i].hasAttackedThisTurn = true;
            loadedUnits[i].movement = 0;
            loadedUnits[i].isVisibleThisTurn = visibleMapTiles[loadedUnits[i].x + loadedUnits[i].y*MAP_W];
        }
    }
    currentTeam = team;
}

bool moveUnitTo(int unitID, int x, int y)
{
    int xDist = abs(x - loadedUnits[unitID].x);
    int yDist = abs(y - loadedUnits[unitID].y);

    if (xDist + yDist > loadedUnits[unitID].movement)
    {
        return false;
    }
    loadedUnits[unitID].x = x;
    loadedUnits[unitID].y = y;
    calculateVisibleTiles(allUnits[loadedUnits[unitID].type].team);
    return true;
}

bool attackUnit(int unitID, int targetUnitID)
{
    struct MUnit curr = loadedUnits[unitID];
    struct MUnit target = loadedUnits[targetUnitID];
    struct Unit u = allUnits[curr.type];

    int xDist = abs(curr.x - target.x);
    int yDist = abs(curr.y - target.y);

    int totalDistance = xDist + yDist;
    if (totalDistance <= 1 || (totalDistance <= 2 && u.type == TYPE_WATER))
    {
        target.health -= u.stats[BUFF_STRENGTH];
        return true;
    }
    return false;
}

void loadUnits(struct UnitSpawn *spawns)
{
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct UnitSpawn us = *(spawns + i);
        loadedUnits[i] = (struct MUnit){us.type, 8, 0, true, false, us.x, us.y};
        // if(us.type == 2)
        // return;
    }
}