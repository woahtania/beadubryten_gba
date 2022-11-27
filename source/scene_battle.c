#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "battlemap.h"
#include "battle.h"
#include "util.h"

#include "sprites.h"

// The actual count of tiles
#define BATTLEMAP_TILES_LEN ((battlemapTilesLen / 64) + 1)

#define CHARBLOCK_MAP 0
#define CHARBLOCK_UNIT 4

#define SCREENBLOCK_MAP 28
#define SCREENBLOCK_UNIT 24

#define CHARBLOCK_MAP 0
#define CHARBLOCK_UNIT 4

#define SCREENBLOCK_MAP 28
#define SCREENBLOCK_UNIT 24

#define UTIL_SPRITE_ID(i) (MAX_UNITS * 3) + i

bool visibleMapTiles[MAP_W * MAP_H];

size_t tile2MapId(size_t tile_x, size_t tile_y)
{
	return (tile_y >= 16 ? (32 * 64 + (tile_y * 2 - 32) * 32) : (tile_y * 64)) +
		   (tile_x >= 16 ? (32 * 32 + (tile_x * 2 - 32)) : tile_x * 2);
}

OBJ_ATTR unit_objs[128];
struct Cursor cursor;

void initUnits()
{
	memcpy(&tile8_mem[CHARBLOCK_UNIT][0], spritesTiles, spritesTilesLen);

	memcpy(pal_obj_mem, spritesPal, spritesPalLen);

	// Initialise an OAM object for each unit
	oam_init(unit_objs, 128); // +1 for utility sprites 🗿

	for (int i = 0; i < MAX_UNITS * 3; i++)
	{
		int tile_id = 4 * loadedUnits[i].type;
		obj_set_attr(&unit_objs[i], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_16, ATTR2_PALBANK(0) | tile_id * 2);
		obj_set_pos(&unit_objs[i], loadedUnits[i].x * 32, loadedUnits[i].y * 32);
	}

	obj_set_attr(&unit_objs[UTIL_SPRITE_ID(0)], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_16, ATTR2_PALBANK(0) | 12 * 8);
	obj_set_pos(&unit_objs[UTIL_SPRITE_ID(0)], 30, 80);

	obj_copy(obj_mem, unit_objs, 128);
}

void initMap()
{
	// Load palette
	memcpy(pal_bg_mem, battlemapPal, battlemapPalLen);
	// Load tiles into CBB 0
	memcpy(&tile8_mem[CHARBLOCK_MAP][0], battlemapTiles, battlemapTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], battlemapMap, battlemapMapLen);

	// Find out the real palette length by looping until black
	u8 pal_len = 0;
	while (battlemapPal[pal_len] != 0)
	{
		pal_len++;
	}

	// Create a fog of war palette by duplicating the current palette and halving the RGB values
	for (u8 i = 0; i < pal_len; i++)
	{
		pal_bg_mem[pal_len + i] = ((pal_bg_mem[i] & 31) / 2) +
								  ((((pal_bg_mem[i] >> 5) & 31) / 2) << 5) +
								  ((((pal_bg_mem[i] >> 10) & 31) / 2) << 10);
	}

	// Create a duplicated tileset
	memcpy(&tile8_mem[CHARBLOCK_MAP][BATTLEMAP_TILES_LEN], battlemapTiles, battlemapTilesLen);
	// Update duplicated tileset to use the fog of war IDs
	u32 *tile = (u32 *)(&tile8_mem[CHARBLOCK_MAP][BATTLEMAP_TILES_LEN]);
	for (size_t i = 0; i < battlemapTilesLen / 4; i++)
	{
		*(tile + i) += (pal_len) + (pal_len << 8) + (pal_len << 16) + (pal_len << 24);
	}
}

void sc_battle_init()
{
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

void updateFog()
{

	for (size_t y = 0; y < MAP_H; y++)
	{
		for (size_t x = 0; x < MAP_W; x++)
		{
			size_t map_id = tile2MapId(x, y);
			if (visibleMapTiles[y * MAP_H + x] && *(&se_mem[SCREENBLOCK_MAP][map_id]) > BATTLEMAP_TILES_LEN)
			{
				*(&se_mem[SCREENBLOCK_MAP][map_id]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id + 1]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id + 32]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id + 33]) -= BATTLEMAP_TILES_LEN;
			}
			else if (!visibleMapTiles[y * MAP_H + x] && *(&se_mem[SCREENBLOCK_MAP][map_id]) <= BATTLEMAP_TILES_LEN)
			{
				*(&se_mem[SCREENBLOCK_MAP][map_id]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id + 1]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id + 32]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[SCREENBLOCK_MAP][map_id + 33]) += BATTLEMAP_TILES_LEN;
			}
		}
	}
}

void updateUnits()
{
	for (int i = 0; i < MAX_UNITS * 3; i++)
	{
		obj_set_pos(&unit_objs[i], loadedUnits[i].x * 32 - (cursor.camX), loadedUnits[i].y * 32 - (cursor.camY));
		if (loadedUnits[i].isVisibleThisTurn)
		{
			// Set regular rendering mode
			obj_unhide(&unit_objs[i], ATTR0_REG);
		}
		else
		{
			// Set disabled rendering mode
			obj_hide(&unit_objs[i]);
		}
	}
	obj_copy(obj_mem, unit_objs, 128);
}

void procKey(int key, int *frameVal, int *changeVal, int delta)
{
	if (key_is_down(key))
	{
		if (*frameVal <= 180)
		{
			if ((*frameVal) % 15 == 0)
				*changeVal += delta;
		}
		else
		{
			if (*frameVal % 5 == 0)
				*changeVal += delta;
		}
		(*frameVal)++;
	}
	else
	{
		*frameVal -= 2;
		if (*frameVal <= 0)
			*frameVal = 0;
	}
}

void sc_battle_tick()
{
	if (key_is_down(KEY_A))
	{
		for (size_t i = 0; i < MAP_W * MAP_H; i++)
		{
			visibleMapTiles[i] = !visibleMapTiles[i];
		}
		updateFog();
	}

	procKey(KEY_DOWN, &cursor.hf_d, &cursor.y, 1);
	procKey(KEY_UP, &cursor.hf_u, &cursor.y, -1);
	procKey(KEY_LEFT, &cursor.hf_l, &cursor.x, -1);
	procKey(KEY_RIGHT, &cursor.hf_r, &cursor.x, 1);

	cursor.x = clamp(cursor.x, 0, MAP_W);
	cursor.y = clamp(cursor.y, 0, MAP_H );
	if (key_hit(KEY_B))
	{
		loadedUnits[0].isVisibleThisTurn = !loadedUnits[0].isVisibleThisTurn;
		updateUnits();
	}
}

void sc_battle_complete()
{
	if (key_is_down(KEY_START))
	{
		switchScene(1);
	}
	obj_set_pos(&unit_objs[UTIL_SPRITE_ID(0)], cursor.x * 16 - cursor.camX, cursor.y * 16 - cursor.camY);

	int cursorScreenPosX;
	int camX = cursor.camX / 16;
	do
	{
		cursorScreenPosX = cursor.x - camX;
		camX--;
	} while (cursorScreenPosX < 2);

	do
	{
		cursorScreenPosX = cursor.x - camX;
		camX++;
	} while (cursorScreenPosX > 11);

	cursor.targetCamX = clamp(camX * 16, 0, 272);

	int diff = abs(cursor.camX - cursor.targetCamX);
	int delta = 1;
	if (diff < 24)
		delta = 1;
	else if (diff < 48)
		delta = 2;
	else
		delta = 4;

	cursor.camX = lerp(cursor.camX, cursor.targetCamX, delta);

	int cursorScreenPosY;
	int camY = cursor.camY / 16;
	do
	{
		cursorScreenPosY = cursor.y - camY;
		camY--;
	} while (cursorScreenPosY < 2);

	do
	{
		cursorScreenPosY = cursor.y - camY;
		camY++;
	} while (cursorScreenPosY > 8);

	cursor.targetCamY = clamp(camY * 16, 0, 352);

	diff = abs(cursor.camY - cursor.targetCamY);
	delta = 1;
	if (diff < 24)
		delta = 1;
	else if (diff < 48)
		delta = 2;
	else
		delta = 4;

	cursor.camY = lerp(cursor.camY, cursor.targetCamY, delta);

	REG_BG0HOFS = cursor.camX;
	REG_BG0VOFS = cursor.camY;
	updateUnits();
}

void sc_battle_deconstruct() {}