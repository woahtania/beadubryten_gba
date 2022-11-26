
//{{BLOCK(map)

//======================================================================
//
//	map, 512x512@8, 
//	Transparent color : FF,00,F6
//	+ palette 256 entries, not compressed
//	+ 19 tiles (t|f reduced) not compressed
//	+ regular map (in SBBs), not compressed, 64x64 
//	Total size: 512 + 1216 + 8192 = 9920
//
//	Time-stamp: 2022-11-26, 00:12:09
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_MAP_H
#define GRIT_MAP_H

#define mapTilesLen 1216
extern const unsigned int mapTiles[304];

#define mapMapLen 8192
extern const unsigned short mapMap[4096];

#define mapPalLen 512
extern const unsigned short mapPal[256];

#endif // GRIT_MAP_H

//}}BLOCK(map)
