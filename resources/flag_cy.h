
//{{BLOCK(flag_cy)

//======================================================================
//
//	flag_cy, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 409 tiles (t|f reduced) not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 26176 + 2048 = 28736
//
//	Time-stamp: 2022-11-26, 11:55:12
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_FLAG_CY_H
#define GRIT_FLAG_CY_H

#define flag_cyTilesLen 26176
extern const unsigned int flag_cyTiles[6544];

#define flag_cyMapLen 2048
extern const unsigned short flag_cyMap[1024];

#define flag_cyPalLen 512
extern const unsigned short flag_cyPal[256];

#endif // GRIT_FLAG_CY_H

//}}BLOCK(flag_cy)
