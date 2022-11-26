#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "map.h"

#include "flag_cy.h"
#include "flag_en.h"
#include "flag_sc.h"


void loadMap()
{
	// Load palette
	memcpy(pal_bg_mem, mapPal, mapPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[0][0], mapTiles, mapTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[30][0], mapMap, mapMapLen);

	// Find out the real palette length by looping until black
	u8 pal_len = 0;
	for (; pal_bg_mem[pal_len] != 0; pal_len++)
	{
	}
	// Create a fog of war palette by duplicating the current palette and halving the RGB values
	for (u8 i = 0; i < pal_len; i++)
	{
		pal_bg_mem[pal_len + i] = ((pal_bg_mem[i] & 31) / 2) +
								  ((((pal_bg_mem[i] >> 5) & 31) / 2) << 5) +
								  ((((pal_bg_mem[i] >> 10) & 31) / 2) << 10);
	}
	u8 tile_len = ((mapTilesLen / 64) + 1) * 2;
	// Create a duplicated tileset
	memcpy(&tile_mem[0][tile_len], mapTiles, mapTilesLen);
	// Update duplicated tileset to use the new palette IDs
	u32 *tile = (u32 *)(&tile_mem[0][tile_len]);
	for (size_t i = 0; i < mapTilesLen / 4; i++)
	{
		*(tile + i) += (pal_len) + (pal_len << 8) + (pal_len << 16) + (pal_len << 24);
	}
	// Load map into SBB 10
	memcpy(&se_mem[10][0], mapMap, mapMapLen);
	u16 *map = &se_mem[10][0];
	for (size_t i = 0; i < mapMapLen / 2; i++)
	{
		*(map + i) += tile_len;
	}
}

void flag_en()
{
	// Load palette
	memcpy(pal_bg_mem, flag_enPal, flag_enPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[0][0], flag_enTiles, flag_enTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[30][0], flag_enMap, flag_enMapLen);
}

void flag_sc()
{
	// Load palette
	memcpy(pal_bg_mem, flag_scPal, flag_scPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[0][0], flag_scTiles, flag_scTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[30][0], flag_scMap, flag_scMapLen);
}

void flag_cy()
{
	// Load palette
	memcpy(pal_bg_mem, flag_cyPal, flag_cyPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[0][0], flag_cyTiles, flag_cyTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[30][0], flag_cyMap, flag_cyMapLen);
}

void sc_battle_init()
{
	REG_DISPCNT = DCNT_MODE1 | DCNT_BG0;
	REG_BG0CNT = BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_64x64;
}

int test = 0;

void sc_battle_tick()
{
	if (test == 0 && key_is_down(KEY_A))
	{
		test = 1;
		loadMap();
	}
}

void sc_battle_complete()
{
	if (key_is_down(KEY_START))
	{
		switchScene(1);
	}
}

void sc_battle_deconstruct() {}