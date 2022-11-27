#include <tonc.h>

#include "panel.h"
#include "ui_panel.h"
#include "scene_battle.h"
#include "battle.h"
#include "sound.h"

// Count of tiles in panel
#define PANEL_TILES 4*5
// The actual count of tiles
#define PANEL_ALLTILES ((ui_panelTilesLen / 64) + 1)

#define PANELLOC_CURHEALTH 2
#define PANELLOC_MAXHEALTH 3
#define PANELLOC_CURWALK 2 + 32
#define PANELLOC_MAXWALK 3 + 32
#define PANELLOC_STR 2 + 64
#define PANELLOC_VIS 2 + 96
#define PANELLOC_FLAG 1 + 128

#define TILELOC_FLAGS 20
#define TILELOC_CURHEALTH 26 
#define TILELOC_MAXHEALTH 34
#define TILELOC_CURWALK 42
#define TILELOC_MAXWALK 50
#define TILELOC_STRVIS 58

#define CHARBLOCK_UI 1
#define SCREENBLOCK_UI 20

int paletteOffset = 0;

void initPanel() {

    // Enable UI background as BG0
    REG_DISPCNT |= DCNT_BG0;
    REG_BG0CNT = BG_CBB(CHARBLOCK_UI) | BG_SBB(SCREENBLOCK_UI) | BG_8BPP | BG_REG_32x32 | BG_PRIO(0);

    REG_BG0VOFS = 80;

    paletteOffset = 0;
    
    // Get offset to palette start
    while(pal_bg_mem[paletteOffset] != 0) {
        paletteOffset++;
    }

    // Load palette at the offset calculated above
	memcpy(pal_bg_mem + paletteOffset, ui_panelPal, 256 - paletteOffset);
	memcpy(&tile8_mem[CHARBLOCK_UI][0], ui_panelTiles, ui_panelTilesLen);
    // Only load tiles for the 4*5 size of the panel
	memcpy(&se_mem[SCREENBLOCK_UI][0], ui_panelMap, 16*PANEL_TILES);

    // For any "emtpy" filler tiles set by grit (0x0005) set them to invisible tile 0
    u16* map = &se_mem[SCREENBLOCK_UI][0];
    for (size_t i = 0; i < ui_panelMapLen/2; i++) {
        if (*(map + i) == 5) *(map + i) = 0;
    }

    // Offset every colour in the tile palette by the paletteOffset calculated above
    // This includes checks for colour 0 which is left as is
    u32* tile = (u32*)(&tile8_mem[CHARBLOCK_UI][0]);
	for (size_t i = 0; i < ui_panelTilesLen/4; i++) {
        if ((*(tile + i) & 0xFF) > 0) {
            *(tile + i) += (paletteOffset);
        }
        if ((*(tile + i) & 0xFF00) > 0) {
            *(tile + i) += (paletteOffset<<8);
        }
        if ((*(tile + i) & 0xFF0000) > 0) {
            *(tile + i) += (paletteOffset<<16);
        }
        if ((*(tile + i) & 0xFF000000) > 0) {
            *(tile + i) += (paletteOffset<<24);
        }
	}
}

void openPanel() {
    int unitId = unitAt(cursor.x, cursor.y);
    // Don't open panel if we're not hovered over a valid unit
    if (unitId == -1) {
        return;
    }

    controlStatus = CONTROL_PANELOPEN;

    // Set panel UI tiles based on unit's stats
    struct MUnit unit = loadedUnits[unitId];
    se_mem[SCREENBLOCK_UI][PANELLOC_CURHEALTH] = TILELOC_CURHEALTH + unit.health-1;
    se_mem[SCREENBLOCK_UI][PANELLOC_MAXHEALTH] = TILELOC_MAXHEALTH + allUnits[unit.type].stats[BUFF_STRENGTH]-1;
    if (unit.movement == 0)
        se_mem[SCREENBLOCK_UI][PANELLOC_CURWALK] = 8;
    else
        se_mem[SCREENBLOCK_UI][PANELLOC_CURWALK] = TILELOC_CURWALK + unit.movement-1;
    se_mem[SCREENBLOCK_UI][PANELLOC_MAXWALK] = TILELOC_MAXWALK + allUnits[unit.type].stats[BUFF_SPEED]-1;
    se_mem[SCREENBLOCK_UI][PANELLOC_STR] = TILELOC_STRVIS + allUnits[unit.type].stats[BUFF_STRENGTH]-1;
    se_mem[SCREENBLOCK_UI][PANELLOC_VIS] = TILELOC_STRVIS + allUnits[unit.type].stats[BUFF_SIGHT]-1;
    se_mem[SCREENBLOCK_UI][PANELLOC_FLAG] = TILELOC_FLAGS + (allUnits[unit.type].team*2);
    se_mem[SCREENBLOCK_UI][PANELLOC_FLAG + 1] = TILELOC_FLAGS + (allUnits[unit.type].team*2) + 1;

    // Scroll panel into view
    REG_BG0HOFS = 34 - SCREEN_WIDTH;
    REG_BG0VOFS = 16 - SCREEN_HEIGHT/2;
    
    // Play open sound
    playSfx(SFX_PANEL_OPEN);
}

void closePanel() {
    // Move panel out of view
    REG_BG0VOFS = 80;
    controlStatus = CONTROL_BATTLEFIELD;

    // Play close sound
    playSfx(SFX_PANEL_CLOSE);
}