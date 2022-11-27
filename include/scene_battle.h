#ifndef SCENE_BATTLE_H
#define SCENE_BATTLE_H

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
};

#endif