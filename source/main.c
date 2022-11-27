#include <tonc.h>
#include <tonc_types.h>
#include <stdbool.h>

#include "scene.h"
#include "scene_battle.h"
#include "scene_main.h"
#include "scene_victory.h"


/**
 * Registers all scene function pointers to the central array
*/
void registerAllScenes() {
	registerScene(SCENE_BATTLE, sc_battle_init, sc_battle_tick, sc_battle_complete, sc_battle_deconstruct);
	registerScene(SCENE_MENU, sc_main_init, sc_main_tick, sc_main_complete, sc_main_deconstruct);
	registerScene(SCENE_VICTORY, sc_victory_init, sc_victory_tick, sc_victory_complete, sc_victory_deconstruct);
}

int main() {
	irq_init(NULL);
	irq_enable(II_VBLANK);
	registerAllScenes();
	currentScene = SCENE_MENU;
	scene_init();

	while(true) {
		scene_tick();
	}

	return 0;
}