
#include "battle.h"

const struct Tile allTiles[12] = {
    {TEAM_NONE, TYPE_LAND, BUFF_NONE, "Plains"},
    {TEAM_NONE, TYPE_WATER, BUFF_NONE, "Sea"},
    {TEAM_NONE, TYPE_AIR, BUFF_NONE, "Mountains"},

    {TEAM_ENGLAND, TYPE_LAND, BUFF_STRENGTH, "Castle"},
    {TEAM_ENGLAND, TYPE_WATER, BUFF_SPEED, "River"},
    {TEAM_ENGLAND, TYPE_AIR, BUFF_SIGHT, "Pennine"},

    {TEAM_CYMRU, TYPE_LAND, BUFF_SPEED, "Valley"},
    {TEAM_CYMRU, TYPE_WATER, BUFF_STRENGTH, "Waterfall"},
    {TEAM_CYMRU, TYPE_AIR, BUFF_SIGHT, "Mountain Range"},

    {TEAM_SCOTLAND, TYPE_LAND, BUFF_SPEED, "Bog"},
    {TEAM_SCOTLAND, TYPE_WATER, BUFF_SIGHT, "Loch"},
    {TEAM_SCOTLAND, TYPE_AIR, BUFF_STRENGTH, "Highland"}};

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

void calculateVisibleTiles(int team)
{
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct Unit u = allUnits[loadedUnits[i].type];
        if (u.team == team)
        {
            int r = u.stats[BUFF_SIGHT];
            while (r > 0)
            {
                int x = r;
                int y = 0;
                int d = 0;
                while (x >= y)
                {
                    visibleMapTiles[(y * MAP_W) + x] = true;
                    d += (2 * y + 1);
                    if (d >= 0)
                    {
                        d += (-2 * x + 1);
                        x--;
                    }
                }
                r--;
            }
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
            loadedUnits[i].isVisibleThisTurn = visibleMapTiles[loadedUnits[i].x + (loadedUnits[i].y * MAP_W)];
        }
    }
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
    if(totalDistance <= 1 || (totalDistance <= 2 && u.type == TYPE_WATER)){
        target.health -= u.stats[BUFF_STRENGTH];
        return true;
    }
    return false;
}