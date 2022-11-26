#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "map.h"

void sc_battle_init()
{
    REG_DISPCNT = DCNT_MODE1 | DCNT_BG0 | DCNT_BG1;
	REG_BG0CNT = BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_64x64;
	REG_BG1CNT = BG_CBB(0) | BG_SBB(10) | BG_8BPP | BG_REG_64x64;


	// Load palette
	memcpy(pal_bg_mem, mapPal, mapPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[0][0], mapTiles, mapTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[30][0], mapMap, mapMapLen);

	// Find out the real palette length by looping until black
	u8 pal_len = 0;
	for (; pal_bg_mem[pal_len] != 0; pal_len++) {}
	// Create a fog of war palette by duplicating the current palette and halving the RGB values
	for (u8 i = 0; i < pal_len; i++) {
		pal_bg_mem[pal_len + i] = ((pal_bg_mem[i] & 31) / 2) + 
			((((pal_bg_mem[i] >> 5) & 31) / 2) << 5) + 
			((((pal_bg_mem[i] >> 10) & 31) / 2) << 10);
	}
	u8 tile_len = ((mapTilesLen/64) + 1)*2;
	// Create a duplicated tileset
	memcpy(&tile_mem[0][tile_len], mapTiles, mapTilesLen);
	// Update duplicated tileset to use the new palette IDs
	u32* tile = (u32*)(&tile_mem[0][tile_len]);
	for (size_t i = 0; i < mapTilesLen/4; i++) {
		*(tile + i) += (pal_len) + (pal_len<<8) + (pal_len<<16) + (pal_len<<24);
	}
	// Load map into SBB 10
	memcpy(&se_mem[10][0], mapMap, mapMapLen);
	u16* map = &se_mem[10][0];
	for (size_t i = 0; i < mapMapLen/2; i++) {
		*(map + i) += tile_len;
	}

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