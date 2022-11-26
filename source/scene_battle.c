#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "battlemap.h"
#include "battle.h"

#include "sprites.h"

// The actual count of tiles
#define BATTLEMAP_TILES_LEN ((battlemapTilesLen/64) + 1)

#define CHARBLOCK_MAP 0
#define CHARBLOCK_UNIT 4

#define SCREENBLOCK_MAP 28
#define SCREENBLOCK_UNIT 24

bool visibleMapTiles[MAP_W * MAP_H];

size_t tile2MapId(size_t tile_x, size_t tile_y) {
	return (tile_y >=16 ? (32*64 + (tile_y*2 - 32)*32) : (tile_y * 64)) + 
		(tile_x >= 16 ? (32*32 + (tile_x*2 - 32)) : tile_x*2);
}

OBJ_ATTR unit_objs[128];

void initUnits() {
	// Load the spritesheet
	memcpy(&tile8_mem[CHARBLOCK_UNIT][0], spritesTiles, spritesTilesLen);
	// Load spritesheet palette
	memcpy(pal_obj_mem, spritesPal, spritesPalLen);

	// Initialise an OAM object for each unit
	oam_init(unit_objs, MAX_UNITS * 3);

	for(int i = 0; i < MAX_UNITS * 3; i++) {
		// Set the tile_id to the tile in the spritesheet for that unit
		int tile_id = 8 * loadedUnits[i].type;
		// Set sprite to a 16x16 square, 256 colour, using palette 0 and the tileID set above
		obj_set_attr(&unit_objs[i], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_16, ATTR2_PALBANK(0) | tile_id);
		// Set position based on unit position
		// TODO: this needs to account for camera position
		obj_set_pos(&unit_objs[i], loadedUnits[i].x * 32, loadedUnits[i].y * 32);
	}
	// Copy all unit objs to vram
	obj_copy(obj_mem, unit_objs, MAX_UNITS * 3);
}

void initMap() {
	// Load palette
	memcpy(pal_bg_mem, battlemapPal, battlemapPalLen);
	// Load tiles into CBB 0
	memcpy(&tile8_mem[CHARBLOCK_MAP][0], battlemapTiles, battlemapTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], battlemapMap, battlemapMapLen);

	// Find out the real palette length by looping until black (thanks grit)
	u8 pal_len = 0;
	while (battlemapPal[pal_len] != 0) {
		pal_len++;
	}

	// Create a fog of war palette by duplicating the current palette and halving the RGB values
	for (u8 i = 0; i < pal_len; i++) {
		pal_bg_mem[pal_len + i] = ((pal_bg_mem[i] & 31) / 2) + 
			((((pal_bg_mem[i] >> 5) & 31) / 2) << 5) + 
			((((pal_bg_mem[i] >> 10) & 31) / 2) << 10);
	}

	// Create a duplicated tileset
	memcpy(&tile8_mem[CHARBLOCK_MAP][BATTLEMAP_TILES_LEN], battlemapTiles, battlemapTilesLen);
	// Update duplicated tileset to use the fog of war IDs
	u32* tile = (u32*)(&tile8_mem[CHARBLOCK_MAP][BATTLEMAP_TILES_LEN]);
	for (size_t i = 0; i < battlemapTilesLen/4; i++) {
		*(tile + i) += (pal_len) + (pal_len<<8) + (pal_len<<16) + (pal_len<<24);
	}
}

void updateFog() {
	for(size_t y = 0; y < MAP_H; y++) {
		for(size_t x = 0; x < MAP_W; x++) {
			size_t map_id = tile2MapId(x, y);
			// If the tile is in vision range and is currently set to fog-of war tile
			if(visibleMapTiles[y * MAP_H + x] && *(&se_mem[SCREENBLOCK_MAP][map_id]) > BATTLEMAP_TILES_LEN) {
				// Reduce tile ids to the non fog-of-war variant
				*(&se_mem[SCREENBLOCK_MAP][map_id]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id+1]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id+32]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id+33]) -= BATTLEMAP_TILES_LEN;
			} 
			// If the tile is out vision range and is currently not set to fog-of war tile
			else if (!visibleMapTiles[y * MAP_H + x] && *(&se_mem[SCREENBLOCK_MAP][map_id]) <= BATTLEMAP_TILES_LEN) {
				// Increase tile ids to the fog-of-war variant
				*(&se_mem[SCREENBLOCK_MAP][map_id]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id+1]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id+32]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id+33]) += BATTLEMAP_TILES_LEN;
			}
		}
	}
}

void updateUnits() {
	for(int i = 0; i < MAX_UNITS * 3; i++) {
		// Set position based on unit position
		// TODO: this needs to account for camera position
		obj_set_pos(&unit_objs[i], loadedUnits[i].x * 32, loadedUnits[i].y * 32);
		// (un)hide unit based on visible status
		if (loadedUnits[i].isVisibleThisTurn) {
			// Set regular rendering mode
			obj_unhide(&unit_objs[i], ATTR0_REG);
		} else {
			// Set disabled rendering mode
			obj_hide(&unit_objs[i]);
		}
	}
	// Copy all unit objs to vram
	obj_copy(obj_mem, unit_objs, MAX_UNITS * 3);
}

void sc_battle_init()
{
	// Set Mode1 (4 backgrounds), enable bg1
    REG_DISPCNT = DCNT_MODE1 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
	REG_BG0CNT = BG_CBB(CHARBLOCK_MAP) | BG_SBB(SCREENBLOCK_MAP) | BG_8BPP | BG_REG_64x64;

	initMap();

	// Create test units
	// for(int i = 0; i < 5; i++) {
	// 	struct MUnit unit = {i % 3 == 0 ? 4 : (i % 2 == 0 ? 11 : 8), 5, 5, false, false, i % 8, i / 8};
	// 	loadedUnits[i] = unit;
	// }
	
	struct MUnit unit = {4, 5, 5, false, false, 3, 3};
	loadedUnits[0] = unit;
	startTurnFor(TEAM_ENGLAND);

	initUnits();

	updateFog();
}


void sc_battle_tick()
{

}

void sc_battle_complete() {
	// Test: flip tile visibility status of every tile and update fog
	if (key_is_down(KEY_A)) {
		for(size_t i = 0; i < MAP_W * MAP_H; i++) {
			visibleMapTiles[i] = !visibleMapTiles[i];
		}
		updateFog();
	}
	// Test: flip unit visibility status of unit 0 and update units
	if (key_hit(KEY_B)) {
		loadedUnits[0].isVisibleThisTurn = !loadedUnits[0].isVisibleThisTurn;
		updateUnits();
	}
}

void sc_battle_complete() {
    if (key_is_down(KEY_START))
    {
        switchScene(1);
    }
}

void sc_battle_deconstruct() {}