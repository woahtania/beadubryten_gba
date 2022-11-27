#include <tonc.h>
#include "scene.h"
#include "battle.h"

#include "victory_cy.h"
#include "victory_en.h"
#include "victory_sc.h"

void sc_victory_init() {
    REG_DISPCNT= DCNT_MODE3 | DCNT_BG2;

    switch ("team who won")
    {
    case TEAM_CYMRU:
        memcpy(vid_mem, victory_cyBitmap, victory_cyBitmapLen);
        break;
    case TEAM_ENGLAND:
        memcpy(vid_mem, victory_enBitmap, victory_enBitmapLen);
        break;
    case TEAM_SCOTLAND:
        memcpy(vid_mem, victory_scBitmap, victory_scBitmapLen);
        break;
    default:
        break;
    }
}

void sc_victory_tick() {
    if (key_hit(KEY_START)) {
        memset(vid_mem, 0, victory_cyBitmapLen);
        switchScene(SCENE_MENU);
    }
}

void sc_victory_complete() {}

void sc_victory_deconstruct() {}