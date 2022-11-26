
//{{BLOCK(flag_en)

//======================================================================
//
//	flag_en, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 29 tiles (t|f reduced) not compressed
//	+ regular map (in SBBs), not compressed, 32x32 
//	Total size: 512 + 1856 + 2048 = 4416
//
//	Time-stamp: 2022-11-26, 11:55:16
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_FLAG_EN_H
#define GRIT_FLAG_EN_H

#define flag_enTilesLen 1856
extern const unsigned int flag_enTiles[464];

#define flag_enMapLen 2048
extern const unsigned short flag_enMap[1024];

#define flag_enPalLen 512
extern const unsigned short flag_enPal[256];

#endif // GRIT_FLAG_EN_H

//}}BLOCK(flag_en)
