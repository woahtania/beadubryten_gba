#ifndef SCENE_BATTLE_H
#define SCENE_BATTLE_H

#define CONTROL_BATTLEFIELD 0
#define CONTROL_UNITMOVE 1
#define CONTROL_ENDTURN 2
#define CONTROL_PANELOPEN 3
#define CONTROL_UNITATK 4

extern struct Cursor cursor;

extern int controlStatus;

void sc_battle_init();

void sc_battle_tick();

void sc_battle_complete();

void sc_battle_deconstruct();

void updateFog();

struct Cursor{
    int x;
    int y;

    int camX;
    int camY;

    int targetCamX;
    int targetCamY;

    // held frames for up down left right
    int hf_l, hf_r, hf_u, hf_d;

    int selectedUnitForMovement;
    int selectedUnitForFrames;
    int selectedUnitForAtk;
};

#endif