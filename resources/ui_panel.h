
//{{BLOCK(ui_panel)

//======================================================================
//
//	ui_panel, 256x256@8, 
//	Transparent color : FF,00,F6
//	+ palette 256 entries, not compressed
//	+ 66 tiles (t reduced) not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 4224 + 2048 = 6784
//
//	Time-stamp: 2022-11-27, 19:44:43
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_UI_PANEL_H
#define GRIT_UI_PANEL_H

#define ui_panelTilesLen 4224
extern const unsigned int ui_panelTiles[1056];

#define ui_panelMapLen 2048
extern const unsigned short ui_panelMap[1024];

#define ui_panelPalLen 512
extern const unsigned short ui_panelPal[256];

#endif // GRIT_UI_PANEL_H

//}}BLOCK(ui_panel)
