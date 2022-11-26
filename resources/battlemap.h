
//{{BLOCK(battlemap)

//======================================================================
//
//	battlemap, 512x512@8, 
//	Transparent color : FF,00,F6
//	+ palette 256 entries, not compressed
//	+ 19 tiles (t|f reduced) not compressed
//	+ regular map (in SBBs), not compressed, 64x64 
//	Total size: 512 + 1216 + 8192 = 9920
//
//	Time-stamp: 2022-11-26, 11:07:31
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_BATTLEMAP_H
#define GRIT_BATTLEMAP_H

#define battlemapTilesLen 1216
extern const unsigned int battlemapTiles[304];

#define battlemapMapLen 8192
extern const unsigned short battlemapMap[4096];

#define battlemapPalLen 512
extern const unsigned short battlemapPal[256];

#endif // GRIT_BATTLEMAP_H

//}}BLOCK(battlemap)
