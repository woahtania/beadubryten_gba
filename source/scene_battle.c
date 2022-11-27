#include <tonc.h>
#include <string.h>
#include "scene.h"
#include "scene_battle.h"
#include "battlemap.h"
#include "battle.h"
#include "util.h"
#include "panel.h"
#include "sound.h"
#include "sprites.h"
#include "flag_en.h"
#include "flag_cy.h"
#include "flag_sc.h"
#include "battlemap_data.h"

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

#define UTIL_SPRITE_ID(i) (MAX_UNITS * 6) + i

int controlStatus = CONTROL_BATTLEFIELD;

bool visibleMapTiles[MAP_W * MAP_H];

// teleport camera for first frame after flag screen
bool instaCamera;

size_t tile2MapId(size_t tile_x, size_t tile_y)
{
	return (tile_y >= 16 ? (32 * 64 + (tile_y * 2 - 32) * 32) : (tile_y * 64)) +
		   (tile_x >= 16 ? (32 * 32 + (tile_x * 2 - 32)) : tile_x * 2);
}

OBJ_ATTR unit_objs[128];
struct Cursor cursor;
struct StoredCamera teamcams[3];
bool frame1 = true;

int health2TileID(int health) {
	// If unit is dead 
	if (health <= 0) {
		// Return last sprite (which should be blank)
		return ATTR2_ID_MASK - 1;
	} 

	return (13 * 8) + ((health-1) * 2);
}

void initUnits() {
	// Load the spritesheet
	memcpy(&tile8_mem[CHARBLOCK_UNIT][0], spritesTiles, spritesTilesLen);
	// Load spritesheet palette
	memcpy(pal_obj_mem, spritesPal, spritesPalLen);

	// Initialise an OAM object for each unit
	oam_init(unit_objs, 128); // +1 for utility sprites 🗿

	for(int i = 0; i < MAX_UNITS * 3; i++) {
		// Set the tile_id to the tile in the spritesheet for that unit
		int tile_id = 8 * loadedUnits[i].type;
		// Set sprite to a 16x16 square, 256 colour, using palette 0 and the tileID set above
		obj_set_attr(&unit_objs[i], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_16, ATTR2_PALBANK(0) | ATTR2_PRIO(2) | tile_id);
		// Set position based on unit position
		// TODO: this needs to account for camera position
		obj_set_pos(&unit_objs[i], 0, 0);

		// Set health indicators
		obj_set_attr(&unit_objs[MAX_UNITS * 3 + i], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_8x8, ATTR2_PALBANK(0)| ATTR2_PRIO(1) | health2TileID(loadedUnits[i].health));
		obj_set_pos(&unit_objs[MAX_UNITS * 3 + i], 0, 0);
	}

	obj_set_attr(&unit_objs[UTIL_SPRITE_ID(0)], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_16, ATTR2_PALBANK(0) | ATTR2_PRIO(0) | 12 * 8);
	obj_set_pos(&unit_objs[UTIL_SPRITE_ID(0)], 80, 30);

	obj_copy(obj_mem, unit_objs, 128);
}

void initMap()
{
	for (int i = 0; i < MAP_W * MAP_H; i++)
	{
		mapTiles[i] = battlemapTileIDs[i];
	}
	// Load palette
	memcpy(pal_bg_mem, battlemapPal, battlemapPalLen);
	// Load tiles into CBB 0
	memcpy(&tile8_mem[CHARBLOCK_MAP][0], battlemapTiles, battlemapTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], battlemapMap, battlemapMapLen);

	// Find out the real palette length by looping until black (thanks grit)
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
		if (loadedUnits[i].health <= 0)
		{
			obj_hide(&unit_objs[i]);
			obj_hide(&unit_objs[MAX_UNITS * 3 + i]);
			continue;
		}
		// Set position based on unit position
		// TODO: Needs to check the bounds of the screen as overflow will cause offscreen units to be rendered
		int obj_x = loadedUnits[i].x * 16 - (cursor.camX);
		int obj_y = loadedUnits[i].y * 16 - (cursor.camY);
		if (obj_x > -16 && obj_x <= SCREEN_WIDTH && obj_y > -16 && obj_y <= SCREEN_HEIGHT) {
			obj_set_pos(&unit_objs[i], obj_x, obj_y);
			obj_set_pos(&unit_objs[MAX_UNITS * 3 + i], obj_x - 3, obj_y - 3);
			// (un)hide unit and health based on visible status
			if (loadedUnits[i].isVisibleThisTurn) {
				// Set regular rendering mode
				obj_unhide(&unit_objs[i], ATTR0_REG);
				obj_unhide(&unit_objs[MAX_UNITS * 3 + i], ATTR0_REG);
				BFN_SET((&unit_objs[MAX_UNITS * 3 + i])->attr2, health2TileID(loadedUnits[i].health), ATTR2_ID);
				//obj_set_attr(&unit_objs[MAX_UNITS * 3 + i], ATTR0_SQUARE | ATTR0_8BPP, ATTR1_SIZE_8x8, ATTR2_PALBANK(0)| ATTR2_PRIO(1) | health2TileID(loadedUnits[i].health));
			} else {
				// Set disabled rendering mode
				obj_hide(&unit_objs[i]);
				obj_hide(&unit_objs[MAX_UNITS * 3 + i]);
			}
		}
		else { 
			obj_hide(&unit_objs[i]);
			obj_hide(&unit_objs[MAX_UNITS * 3 + i]);
		}

	}
	// Copy all unit objs to vram
	obj_copy(obj_mem, unit_objs, 128);
}

void sc_battle_init()
{
	cursor.selectedUnitForMovement = -1;
	cursor.selectedUnitForAtk = -1;
	// Set Mode1 (4 backgrounds), enable bg1
    REG_DISPCNT = DCNT_MODE1 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;

	REG_BG1CNT = BG_CBB(CHARBLOCK_MAP) | BG_SBB(SCREENBLOCK_MAP) | BG_8BPP | BG_REG_64x64 | BG_PRIO(3);

	initMap();
	initPanel();
	initSound();

	loadUnits(&battlemapSpawns);

	// initialise cursor positions
	for (int i = 0; i < MAX_UNITS * 3; i++)
	{
		struct MUnit mu = loadedUnits[i];
		struct Unit u = allUnits[mu.type];

		if (u.isSignatureUnit)
		{
			teamcams[u.team].x = mu.x;
			teamcams[u.team].y = mu.y;
		}
	}

	startTurnFor(TEAM_SCOTLAND);
	changeSong(MOD_SCOTLAND);
	quietSong();

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
	cursor = (struct Cursor){teamcams[TEAM_ENGLAND].x, teamcams[TEAM_ENGLAND].y, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1};
	instaCamera = true;
}

void flag_sc()
{
	// Load palette
	memcpy(pal_bg_mem, flag_scPal, flag_scPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[CHARBLOCK_MAP][0], flag_scTiles, flag_scTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], flag_scMap, flag_scMapLen);
	cursor = (struct Cursor){teamcams[TEAM_SCOTLAND].x, teamcams[TEAM_SCOTLAND].y, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1};
	instaCamera = true;
}

void flag_cy()
{
	// Load palette
	memcpy(pal_bg_mem, flag_cyPal, flag_cyPalLen);
	// Load tiles into CBB 0
	memcpy(&tile_mem[CHARBLOCK_MAP][0], flag_cyTiles, flag_cyTilesLen);
	// Load map into SBB 30
	memcpy(&se_mem[SCREENBLOCK_MAP][0], flag_cyMap, flag_cyMapLen);
	cursor = (struct Cursor){teamcams[TEAM_CYMRU].x, teamcams[TEAM_CYMRU].y, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, -1};
	instaCamera = true;
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

void updateCamera() {
	int oldX = cursor.x;
	int oldY = cursor.y;

	procKey(KEY_DOWN, &cursor.hf_d, &cursor.y, 1);
	procKey(KEY_UP, &cursor.hf_u, &cursor.y, -1);
	procKey(KEY_LEFT, &cursor.hf_l, &cursor.x, -1);
	procKey(KEY_RIGHT, &cursor.hf_r, &cursor.x, 1);

	if (controlStatus == CONTROL_UNITATK)
	{
		struct MUnit mu = loadedUnits[cursor.selectedUnitForAtk];
		int atkDist = allUnits[mu.type].type == TYPE_WATER ? 2 : 1;
		cursor.x = clamp(cursor.x, mu.x - atkDist, mu.x + atkDist + 1);
		cursor.y = clamp(cursor.y, mu.y - atkDist, mu.y + atkDist + 1);
	}

	cursor.x = clamp(cursor.x, 0, MAP_W);
	cursor.y = clamp(cursor.y, 0, MAP_H );

	if (cursor.x != oldX || cursor.y != oldY) {
		playSfx(SFX_CURSOR_CLICK);
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

	if (instaCamera)
	{
		cursor.camX = cursor.targetCamX;
		cursor.camY = cursor.targetCamY;
		instaCamera = false;
	}

	REG_BG1HOFS = cursor.camX;
	REG_BG1VOFS = cursor.camY;
}

void sc_battle_tick()
{
	bool a_hit = key_hit(KEY_A);
	bool b_hit = key_hit(KEY_B);
	bool r_hit = key_hit(KEY_R);

	if (r_hit && controlStatus == CONTROL_BATTLEFIELD) {
		openPanel();
		r_hit = false;
	}

	if (r_hit && controlStatus == CONTROL_PANELOPEN) {
		closePanel();
		r_hit = false;
	}

	if (a_hit && controlStatus == CONTROL_BATTLEFIELD)
	{
		a_hit = false;
		for (int i = 0; i < MAX_UNITS * 3; i++)
		{
			int unit = unitAt(cursor.x, cursor.y);
			if (unit != -1 && loadedUnits[unit].movement > 0) {
				cursor.selectedUnitForMovement = unit;
				controlStatus = CONTROL_UNITMOVE;
				cursor.selectedUnitForFrames = 0;
				playSfx(SFX_MOVE_SELECT);
			}
		}
	}

	if (a_hit && controlStatus == CONTROL_UNITMOVE)
	{
		a_hit = false;
		if (moveUnitTo(cursor.selectedUnitForMovement, cursor.x, cursor.y))
		{
			loadedUnits[cursor.selectedUnitForMovement].isVisibleThisTurn = true;
			cursor.selectedUnitForMovement = -1;
			cursor.selectedUnitForFrames = 0;
			controlStatus = CONTROL_BATTLEFIELD;
			updateFog();
		}
		else
		{
			playSfx(SFX_CANCEL);
		}
	}

	if (b_hit && (controlStatus == CONTROL_BATTLEFIELD))
	{
		b_hit = false;
		for (int i = 0; i < MAX_UNITS * 3; i++)
		{
			struct MUnit mu = loadedUnits[i];
			if (mu.x == cursor.x && mu.y == cursor.y)
			{
				if (!mu.hasAttackedThisTurn)
				{
					cursor.selectedUnitForAtk = i;
					cursor.selectedUnitForFrames = 0;
					controlStatus = CONTROL_UNITATK;
					playSfx(SFX_BATTLE_SELECT);
				}
			}
		}
	}

	if (a_hit && (controlStatus == CONTROL_UNITATK))
	{
		a_hit = false;
		for (int i = 0; i < MAX_UNITS * 3; i++)
		{
			if (loadedUnits[i].x == cursor.x && loadedUnits[i].y == cursor.y)
			{
				if (attackUnit(cursor.selectedUnitForAtk, i))
				{
					controlStatus = CONTROL_BATTLEFIELD;
					cursor.selectedUnitForAtk = -1;
					cursor.selectedUnitForFrames = 0;
				} else {
					playSfx(SFX_CANCEL);
				}
				break;
			}
		}
	}

	if (controlStatus == CONTROL_UNITMOVE)
	{
		cursor.selectedUnitForFrames++;
		if (cursor.selectedUnitForFrames % 60 < 30)
		{
			loadedUnits[cursor.selectedUnitForMovement].isVisibleThisTurn = false;
		}
		else
		{
			loadedUnits[cursor.selectedUnitForMovement].isVisibleThisTurn = true;
		}
	}

	if (controlStatus == CONTROL_UNITATK)
	{
		cursor.selectedUnitForFrames++;
		if (cursor.selectedUnitForFrames % 60 < 30)
		{
			obj_hide(&unit_objs[UTIL_SPRITE_ID(0)]);
		}
		else
		{
			obj_unhide(&unit_objs[UTIL_SPRITE_ID(0)], ATTR0_REG);
		}
	}

	if (b_hit && (controlStatus == CONTROL_UNITATK || controlStatus == CONTROL_UNITMOVE))
	{
		b_hit = false;
		controlStatus = CONTROL_BATTLEFIELD;
		if (cursor.selectedUnitForMovement != -1)
			loadedUnits[cursor.selectedUnitForMovement].isVisibleThisTurn = true;
		cursor.selectedUnitForAtk = -1;
		cursor.selectedUnitForMovement = -1;
		cursor.selectedUnitForFrames = 0;
		playSfx(SFX_CANCEL);
	}

	if (controlStatus == CONTROL_BATTLEFIELD)
	{
		obj_unhide(&unit_objs[UTIL_SPRITE_ID(0)], ATTR0_REG);
	}

	updateSound();
}

bool flag_display = false;

void sc_battle_complete() {
	if ((key_hit(KEY_START) && controlStatus == CONTROL_BATTLEFIELD) || frame1 || (controlStatus == CONTROL_ENDTURN && key_hit(KEY_A)))
	{
		frame1 = false;
		if(!flag_display) {
			// Hide all sprites
			for(int i = 0; i < MAX_UNITS * 3; i++) {
				loadedUnits[i].isVisibleThisTurn = false;
			}
			// Reset camera
			REG_BG1HOFS = 0;
			REG_BG1VOFS = 0;
			cursor = (struct Cursor){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1};
      
			controlStatus = CONTROL_ENDTURN;
			// Show flag and play song
			switch (currentTeam)
			{
			case TEAM_ENGLAND:
				flag_cy();
				changeSong(MOD_CYMRU);
				break;
			case TEAM_CYMRU:
				flag_sc();
				changeSong(MOD_SCOTLAND);
				break;
			case TEAM_SCOTLAND:
				flag_en();
				changeSong(MOD_ENGLAND);
				break;
			default:
				break;
			}
			// Hide panel layer BG
			REG_DISPCNT &= ~DCNT_BG0;
		} else {
			initMap();		
			initPanel();
			startTurnFor((currentTeam + 1) % 3);
			updateFog();			controlStatus = CONTROL_BATTLEFIELD;
			quietSong();
		}
		flag_display = !flag_display;
	}
	// Don't allow camera movement when the end turn display is up or a panel is open
	if (!flag_display && controlStatus != CONTROL_PANELOPEN) {
		updateCamera();
	}

	updateUnits();
}

void sc_battle_deconstruct() {}