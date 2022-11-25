
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
     (int[3]){8, 6, 6}, true},

    {"Barbary Lion", TEAM_ENGLAND, TYPE_LAND,
     (int[3]){6, 6, 8}, true},

    {"Loch Ness Monster", TEAM_SCOTLAND, TYPE_WATER,
     (int[3]){6, 8, 6}, true},

    {"Longboat", TEAM_SCOTLAND, TYPE_WATER,
     (int[3]){4, 3, 4}, true}, // total 11
    {"Clansmen", TEAM_SCOTLAND, TYPE_LAND,
     (int[3]){2, 3, 1}, true}, // total 6
    {"Teuchtars", TEAM_SCOTLAND, TYPE_AIR,
     (int[3]){3, 2, 1}, true}, // total 6

    {"Mountaineers", TEAM_ENGLAND, TYPE_AIR,
     (int[3]){3, 1, 2}, true}, // total 6
    {"Privateers", TEAM_ENGLAND, TYPE_WATER,
     (int[3]){2, 2, 2}, true}, // total 6
    {"Cavalry", TEAM_ENGLAND, TYPE_LAND,
     (int[3]){3, 3, 5}, true}, // total 11

    {"Rangers", TEAM_CYMRU, TYPE_AIR,
     (int[3]){3, 5, 3}, true}, // total 11
    {"Fishermen", TEAM_CYMRU, TYPE_WATER,
     (int[3]){1, 2, 3}, true}, // total 6
    {"Spearmen", TEAM_CYMRU, TYPE_LAND,
     (int[3]){1, 3, 2}, true} // total 6
};