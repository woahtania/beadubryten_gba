#ifndef SFX_H
#define SFX_H

#include "soundbank.h"
#include "maxmod.h"

void initSound();

void updateSound();

void playSfx(mm_word sfxId);

void changeSong(mm_word songId);

void quietSong();

#endif
