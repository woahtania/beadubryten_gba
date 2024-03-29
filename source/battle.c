#include "battle.h"
#include "sound.h"

struct MUnit loadedUnits[MAX_UNITS * 3];
int mapTiles[MAP_W * MAP_H];
int currentTeam;

const struct Tile allTiles[12] = {
    {TEAM_ENGLAND, TYPE_LAND, BUFF_STRENGTH, 1, "Castle"},
    {TEAM_ENGLAND, TYPE_WATER, BUFF_SPEED, 2, "River"},
    {TEAM_ENGLAND, TYPE_AIR, BUFF_SIGHT, 1, "Pennine"},

    {TEAM_SCOTLAND, TYPE_LAND, BUFF_SPEED, 1, "Bog"},
    {TEAM_SCOTLAND, TYPE_WATER, BUFF_SIGHT, 1, "Loch"},
    {TEAM_SCOTLAND, TYPE_AIR, BUFF_STRENGTH, 2, "Highland"},

    {TEAM_CYMRU, TYPE_LAND, BUFF_SPEED, 1, "Valley"},
    {TEAM_CYMRU, TYPE_WATER, BUFF_STRENGTH, 1, "Waterfall"},
    {TEAM_CYMRU, TYPE_AIR, BUFF_SIGHT, 2, "Mountain Range"},

    {TEAM_NONE, TYPE_AIR, BUFF_NONE, 0, "Mountains"},
    {TEAM_NONE, TYPE_LAND, BUFF_NONE, 0, "Plains"},
    {TEAM_NONE, TYPE_WATER, BUFF_NONE, 0, "Sea"}};

const struct Unit allUnits[12] = {
    {"Y Ddraig Goch", TEAM_CYMRU, TYPE_AIR,
     (int[3]){8, 6, 6}, true, 6},

    {"Barbary Lion", TEAM_ENGLAND, TYPE_LAND,
     (int[3]){6, 6, 8}, true, 10},

    {"Loch Ness Monster", TEAM_SCOTLAND, TYPE_WATER,
     (int[3]){6, 8, 6}, true, 10},

    {"Longboat", TEAM_SCOTLAND, TYPE_WATER,
     (int[3]){4, 3, 4}, false, 4}, // total 11
    {"Clansmen", TEAM_SCOTLAND, TYPE_LAND,
     (int[3]){2, 3, 1}, false, 5}, // total 6
    {"Teuchtars", TEAM_SCOTLAND, TYPE_AIR,
     (int[3]){3, 2, 1}, false, 2}, // total 6

    {"Mountaineers", TEAM_ENGLAND, TYPE_AIR,
     (int[3]){3, 1, 2}, false, 2}, // total 6
    {"Privateers", TEAM_ENGLAND, TYPE_WATER,
     (int[3]){2, 2, 2}, false, 4}, // total 6
    {"Cavalry", TEAM_ENGLAND, TYPE_LAND,
     (int[3]){3, 3, 5}, false, 5}, // total 11

    {"Rangers", TEAM_CYMRU, TYPE_AIR,
     (int[3]){3, 5, 3}, false, 2}, // total 11
    {"Fishermen", TEAM_CYMRU, TYPE_WATER,
     (int[3]){1, 2, 3}, false, 4}, // total 6
    {"Spearmen", TEAM_CYMRU, TYPE_LAND,
     (int[3]){1, 3, 2}, false, 5} // total 6
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
            int x = loadedUnits[i].x;
            int y = loadedUnits[i].y;
            struct Tile tile = allTiles[mapTiles[(y * MAP_W) + x] - 1];
            int sightModifier = 0;
            if (tile.buffType == BUFF_SIGHT)
            {
                sightModifier += (tile.team == team) ? tile.buffStrength : -(tile.buffStrength);
            }

            int r = u.stats[BUFF_SIGHT] + sightModifier;

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

int unitAt(int x, int y) {
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct MUnit mu = loadedUnits[i];
        if (mu.x == x && mu.y == y && mu.isVisibleThisTurn)
        {
            return i;
        }
    }
    return -1;
}

void startTurnFor(int team)
{
    calculateVisibleTiles(team);
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct Unit u = allUnits[loadedUnits[i].type];
        if (loadedUnits[i].health <= 0)
        {
            loadedUnits[i].isVisibleThisTurn = false;
            loadedUnits[i].hasAttackedThisTurn = true;
            loadedUnits[i].movement = 0;
        }
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
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct MUnit m = loadedUnits[i];
        if (m.x == x && m.y == y && m.health >= 1)
            return false;
    }

    struct MUnit curr = loadedUnits[unitID];
    struct Unit u = allUnits[curr.type];
    int xDist = x - curr.x;
    int yDist = y - curr.y;

    struct Tile tile = allTiles[mapTiles[(y * MAP_W) + x] - 1];

    if (u.type == TYPE_WATER && tile.type != TYPE_WATER)
    {
        return false;
    }

    if (u.isSignatureUnit && u.team == TEAM_SCOTLAND)
    {
        // nessy go hog wild
        if (visibleMapTiles[(y * MAP_W) + x])
        {
            if (tile.type == TYPE_WATER)
            {
                loadedUnits[unitID].x = x;
                loadedUnits[unitID].y = y;
                loadedUnits[unitID].movement = 0;

                return true;
            }
        }
    }

    int totalDistance = abs(xDist + yDist);
    int speedBuff = 0;
    int movementRefund = 0; // used for draig

    while (xDist != 0)
    {
        struct Tile tile = allTiles[mapTiles[(curr.y * MAP_W) + (curr.x + xDist)] - 1];
        if (u.type != TYPE_AIR && tile.type == TYPE_AIR)
            return false;
        if (tile.type == TYPE_AIR && u.isSignatureUnit && u.team == TEAM_CYMRU)
            movementRefund++;
        if (tile.buffType == BUFF_SPEED)
        {
            if (tile.team == u.team)
                speedBuff += tile.buffStrength;
            else
                speedBuff -= tile.buffStrength;
        }
        if (xDist < 0)
            xDist++;
        if (xDist > 0)
            xDist--;
    }

    while (yDist != 0)
    {
        struct Tile tile = allTiles[mapTiles[((curr.y + yDist) * MAP_W) + (curr.x)] - 1];
        if (u.type != TYPE_AIR && tile.type == TYPE_AIR)
            return false;
        if (tile.type == TYPE_AIR && u.isSignatureUnit && u.team == TEAM_CYMRU)
            movementRefund++;
        if (tile.buffType == BUFF_SPEED)
        {
            if (tile.team == u.team)
                speedBuff += tile.buffStrength;
            else
                speedBuff -= tile.buffStrength;
        }
        if (yDist < 0)
            yDist++;
        if (yDist > 0)
            yDist--;
    }
    if (speedBuff != 0)
        speedBuff = (curr.movement / speedBuff) / 2;

    if (totalDistance > loadedUnits[unitID].movement + speedBuff + movementRefund)
    {
        return false;
    }
    loadedUnits[unitID].x = x;
    loadedUnits[unitID].y = y;
    loadedUnits[unitID].movement -= totalDistance - movementRefund;
    loadedUnits[unitID].movement = clamp(loadedUnits[unitID].movement, 0, 8);
    calculateVisibleTiles(allUnits[loadedUnits[unitID].type].team);
    return true;
}

bool attackUnit(int unitID, int targetUnitID)
{
    struct MUnit curr = loadedUnits[unitID];
    struct MUnit target = loadedUnits[targetUnitID];
    struct Unit u = allUnits[curr.type];
    struct Unit tu = allUnits[target.type];

    if (u.team == tu.team)
        return false;

    int xDist = abs(curr.x - target.x);
    int yDist = abs(curr.y - target.y);

    int totalDistance = xDist + yDist;
    if (totalDistance <= 1 || (totalDistance <= 2 && u.type == TYPE_WATER))
    {
        target.health -= u.stats[BUFF_STRENGTH];
        struct Tile currentTile = allTiles[mapTiles[(curr.y * MAP_W) + curr.x]];

        int damage = u.stats[BUFF_STRENGTH];
        if (currentTile.buffType == BUFF_STRENGTH && currentTile.team == u.team)
        {
            damage += currentTile.buffStrength;
        }
        else if (currentTile.buffType == BUFF_STRENGTH && currentTile.team != u.team)
        {
            damage -= currentTile.buffStrength;
        }

        loadedUnits[targetUnitID].health = clamp(target.health - clamp(damage, 0, 8), 0, 8);
        if (loadedUnits[targetUnitID].health <= 0)
            loadedUnits[targetUnitID].isVisibleThisTurn = false;
        playSfx(SFX_ATTACK);
        loadedUnits[targetUnitID].hasAttackedThisTurn = true;
        return true;
    }
    return false;
}

void loadUnits(struct UnitSpawn *spawns)
{
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct UnitSpawn us = *(spawns + i);
        loadedUnits[i] = (struct MUnit){us.type, allUnits[us.type].stats[BUFF_STRENGTH], 0, true, false, us.x, us.y};
    }
}

bool canTeamContinue(int team)
{
    int totalUnits;
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct MUnit mu = loadedUnits[i];
        struct Unit u = allUnits[mu.type];
        if (mu.health >= 1 && u.team == team)
            totalUnits++;
    }
    return totalUnits >= 1;
}

int hasTeamWon()
{
    bool teamsWithSignature[3];
    for (int i = 0; i < MAX_UNITS * 3; i++)
    {
        struct MUnit mu = loadedUnits[i];
        struct Unit u = allUnits[mu.type];
        if (u.isSignatureUnit && mu.health >= 1)
            teamsWithSignature[u.team] = true;
    }
    if (teamsWithSignature[TEAM_ENGLAND] && !teamsWithSignature[TEAM_CYMRU] && !teamsWithSignature[TEAM_SCOTLAND])
        return TEAM_ENGLAND;
    if (!teamsWithSignature[TEAM_ENGLAND] && teamsWithSignature[TEAM_CYMRU] && !teamsWithSignature[TEAM_SCOTLAND])
        return TEAM_CYMRU;
    if (!teamsWithSignature[TEAM_ENGLAND] && !teamsWithSignature[TEAM_CYMRU] && teamsWithSignature[TEAM_SCOTLAND])
        return TEAM_SCOTLAND;

    return -1;
}