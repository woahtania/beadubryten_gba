#include "scene.h"
#include <tonc_bios.h>
#include <tonc.h>
#include <maxmod.h>

// list of all scenes
struct Scene _allScenes[3];
int currentScene;

void registerScene(int index, void (*init)(), void(*tick)(), void(*complete)(), void(*deconstruct)()){
    struct Scene sc;
    sc.tick = tick;
    sc.complete = complete;
    sc.init = init;
    sc.deconstruct = deconstruct;
    _allScenes[index] = sc;
}

void switchScene(int scene){
    _allScenes[currentScene].deconstruct();
    _allScenes[scene].init();
    currentScene = scene;
}

void scene_init(){
    _allScenes[currentScene].init();
}

void scene_tick(){
    key_poll(); // Update help keys
    _allScenes[currentScene].tick();
    // Wait for new frame (SWI 5)
    VBlankIntrWait();
    _allScenes[currentScene].complete();
}
