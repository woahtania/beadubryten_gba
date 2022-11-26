
//{{BLOCK(flag_sc)

//======================================================================
//
//	flag_sc, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 38 tiles (t|f reduced) not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 2432 + 2048 = 4992
//
//	Time-stamp: 2022-11-26, 11:55:09
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_FLAG_SC_H
#define GRIT_FLAG_SC_H

#define flag_scTilesLen 2432
extern const unsigned int flag_scTiles[608];

#define flag_scMapLen 2048
extern const unsigned short flag_scMap[1024];

#define flag_scPalLen 512
extern const unsigned short flag_scPal[256];

#endif // GRIT_FLAG_SC_H

//}}BLOCK(flag_sc)
