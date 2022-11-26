#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "battlemap.h"
#include "battle.h"

// The actual count of tiles
#define BATTLEMAP_TILES_LEN ((battlemapTilesLen / 64) + 1)

bool visibleMapTiles[MAP_W * MAP_H];

void loadMap()
{
	// Load palette
	memcpy(pal_bg_mem, battlemapPal, battlemapPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[0][0], battlemapTiles, battlemapTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[30][0], battlemapMap, battlemapMapLen);

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
	memcpy(&tile8_mem[0][BATTLEMAP_TILES_LEN], battlemapTiles, battlemapTilesLen);
	// Update duplicated tileset to use the fog of war IDs
	u32 *tile = (u32 *)(&tile8_mem[0][BATTLEMAP_TILES_LEN]);
	for (size_t i = 0; i < battlemapTilesLen / 4; i++)
	{
		*(tile + i) += (pal_len) + (pal_len << 8) + (pal_len << 16) + (pal_len << 24);
	}
}

void sc_battle_init()
{
	REG_DISPCNT = DCNT_MODE1 | DCNT_BG0;
	REG_BG0CNT = BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_64x64;

	// for(size_t i = 0; i < MAP_W * MAP_H; i++) {
	// 	visibleMapTiles[i] = true;
	// }

	// for(size_t i = 0; i < 32; i++) {
	// 	visibleMapTiles[i*MAP_W + 16] = false;
	// }

	loadMap();

	struct MUnit unit = {1, 5, 5, false, false, 10, 10};
	loadedUnits[0] = unit;
	startTurnFor(TEAM_ENGLAND);

	updateFog();
}

size_t tile2MapId(size_t tile_x, size_t tile_y)
{
	return (tile_y >= 16 ? (32 * 64 + (tile_y * 2 - 32) * 32) : (tile_y * 64)) +
		   (tile_x >= 16 ? (32 * 32 + (tile_x * 2 - 32)) : tile_x * 2);
}

void updateFog()
{

	for (size_t y = 0; y < MAP_H; y++)
	{
		for (size_t x = 0; x < MAP_W; x++)
		{
			size_t map_id = tile2MapId(x, y);
			if (visibleMapTiles[y * MAP_H + x] && *(&se_mem[30][map_id]) > BATTLEMAP_TILES_LEN)
			{
				*(&se_mem[30][map_id]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[30][map_id + 1]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[30][map_id + 32]) -= BATTLEMAP_TILES_LEN;
				*(&se_mem[30][map_id + 33]) -= BATTLEMAP_TILES_LEN;
			}
			else if (!visibleMapTiles[y * MAP_H + x] && *(&se_mem[30][map_id]) <= BATTLEMAP_TILES_LEN)
			{
				*(&se_mem[30][map_id]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[30][map_id + 1]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[30][map_id + 32]) += BATTLEMAP_TILES_LEN;
				*(&se_mem[30][map_id + 33]) += BATTLEMAP_TILES_LEN;
			}
		}
	}
}

struct Cursor cursor;

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
		*frameVal-=2;
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
	cursor.y = clamp(cursor.y, 0, MAP_H - 10);
}

void sc_battle_complete()
{
	if (key_is_down(KEY_START))
	{
		switchScene(1);
	}
	REG_BG0HOFS = cursor.x * 16;
	REG_BG0VOFS = cursor.y * 16;
}

void sc_battle_deconstruct() {}