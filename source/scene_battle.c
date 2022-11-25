
#include "scene_battle.h"
#include <tonc.h>
#include "scene.h"

void sc_battle_init()
{
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
	REG_BG0CNT = BG_CBB(0) | BG_SBB(30) | BG_4BPP;


}

void sc_battle_tick()
{
    
}

void sc_battle_complete() {
    if (key_is_down(KEY_START))
    {
        switchScene(1);
    }
}

void sc_battle_deconstruct() {}