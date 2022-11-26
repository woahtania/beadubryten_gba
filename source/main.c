#include <tonc.h>
#include <tonc_types.h>
#include <stdbool.h>

#include "scene.h"
#include "scene_battle.h"

/**
 * Registers all scene function pointers to the central array
*/
void registerAllScenes() {
	registerScene(SCENE_BATTLE, sc_battle_init, sc_battle_tick, sc_battle_complete, sc_battle_deconstruct);
}

int main() {
	irq_init(NULL);
	irq_enable(II_VBLANK);
	registerAllScenes();
	currentScene = SCENE_BATTLE;
	scene_init();

	while(true) {
		scene_tick();
	}

	return 0;
}