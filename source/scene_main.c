#include "tonc.h"
#include "menu_start.h"
#include "menu_guide.h"
#include "scene.h"

#include "guide_1.h"
#include "guide_2.h"
#include "guide_3.h"
#include "guide_4.h"
#include "guide_5.h"
#include "guide_6.h"
#include "guide_7.h"
#include "guide_8.h"
#include "guide_9.h"
#include "guide_10.h"
#include "guide_11.h"

int* guideBitmaps[11] = {
    guide_1Bitmap,
    guide_2Bitmap,
    guide_3Bitmap,
    guide_4Bitmap,
    guide_5Bitmap,
    guide_6Bitmap,
    guide_7Bitmap,
    guide_8Bitmap,
    guide_9Bitmap,
    guide_10Bitmap,
    guide_11Bitmap
};

int guideLens[11] = {
    guide_1BitmapLen,
    guide_2BitmapLen,
    guide_3BitmapLen,
    guide_4BitmapLen,
    guide_5BitmapLen,
    guide_6BitmapLen,
    guide_7BitmapLen,
    guide_8BitmapLen,
    guide_9BitmapLen,
    guide_10BitmapLen,
    guide_11BitmapLen
};


bool isGuide = false;
int guidePage = -1;

void sc_main_init() {
    REG_DISPCNT= DCNT_MODE3 | DCNT_BG2;

    memcpy(vid_mem, menu_startBitmap, menu_startBitmapLen);
}

void sc_main_tick() {
    if (key_hit(KEY_UP) || key_hit(KEY_DOWN)) {
        if (isGuide) 
            memcpy(vid_mem, menu_startBitmap, menu_startBitmapLen);
        else
            memcpy(vid_mem, menu_guideBitmap, menu_guideBitmapLen);
        isGuide = !isGuide;
    }
    if (key_hit(KEY_A) || key_hit(KEY_START)) {
        if (guidePage != -1 || isGuide) {
            guidePage += 1;
            if (guidePage == 11) {
                guidePage = -1;
                isGuide = false;
                memcpy(vid_mem, menu_startBitmap, menu_startBitmapLen);
            } else {
                memcpy(vid_mem, guideBitmaps[guidePage], guideLens[guidePage]);
            }
        } else {
            memset(vid_mem, 0, menu_startBitmapLen);
            switchScene(SCENE_BATTLE);
        }
    }
}

void sc_main_complete() {}

void sc_main_deconstruct() {}
