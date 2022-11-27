#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "battlemap.h"
#include "battle.h"

#include "sprites.h"
#include "flag_en.h"
#include "flag_cy.h"
#include "flag_sc.h"
#include "battlemap_data.h"

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

#include "flag_cy.h"
#include "flag_en.h"
#include "flag_sc.h"


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
		// TODO: Needs to check the bounds of the screen as overflow will cause offscreen units to be rendered
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

	loadUnits(&battlemapSpawns);
	startTurnFor(TEAM_SCOTLAND);

	initUnits();

	updateFog();
}

void flag_en()
{
	// Load palette
	memcpy(pal_bg_mem, flag_enPal, flag_enPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[CHARBLOCK_MAP][0], flag_enTiles, flag_enTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], flag_enMap, flag_enMapLen);

}

void flag_sc()
{
	// Load palette
	memcpy(pal_bg_mem, flag_scPal, flag_scPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[CHARBLOCK_MAP][0], flag_scTiles, flag_scTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], flag_scMap, flag_scMapLen);
}

void flag_cy()
{
	// Load palette
	memcpy(pal_bg_mem, flag_cyPal, flag_cyPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[CHARBLOCK_MAP][0], flag_cyTiles, flag_cyTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], flag_cyMap, flag_cyMapLen);
}

void sc_battle_tick()
{

}

bool flag_display = false;

void sc_battle_complete() {
	// Test: flip tile visibility status of every tile and update fog
	if (key_is_down(KEY_R)) {
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
	// Test: cycle turn between teams
	if (key_hit(KEY_A)) {
		if(!flag_display) {
			// Hide all sprites
			for(int i = 0; i < MAX_UNITS * 3; i++) {
				obj_hide(&unit_objs[i]);
			}
			obj_copy(obj_mem, unit_objs, MAX_UNITS * 3);
			// Show flag
			switch (currentTeam)
			{
			case TEAM_ENGLAND:
				flag_cy();
				break;
			case TEAM_CYMRU:
				flag_sc();
				break;
			case TEAM_SCOTLAND:
				flag_en();
				break;
			default:
				break;
			}
		} else {
			// TODO: This is yoinked from init_map to avoid duplicating palette/tileset repeatedly, should probably be a function of its own
			initMap();

			startTurnFor((currentTeam + 1) % 3);
			updateUnits();
			updateFog();
		}
		flag_display = !flag_display;
	}
}

void sc_battle_deconstruct() {}