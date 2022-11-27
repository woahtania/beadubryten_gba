#include "sound.h"
#include "soundbank_bin.h"
#include "tonc.h"

void initSound() {
    irq_add(II_VBLANK, mmVBlank);
    mmInitDefault(soundbank_bin, 8);
    
}

void updateSound() {
    mmFrame();
}

void playSfx(mm_word sfxId) {
    mmEffect(sfxId);
}

