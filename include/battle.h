#ifndef BATTLE_H
#define BATTLE_H

#include <string.h>
#include <stdbool.h>

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

struct Tile{
    int team; // one of the defined teams above or -1 for generic
    int type; // the type of tile (land, water, or air)
    int buffType; // the type of buff applied to home team and debuff applied to home team
    int buffStrength = 1;
    char* name;
};

struct Unit{
    char* name;
    int team;
    int type;
    int stats[3]; // stats
    bool isSignatureUnit;
};

extern const struct Unit allUnits[12];

extern const struct Tile allTiles[12];

#endif