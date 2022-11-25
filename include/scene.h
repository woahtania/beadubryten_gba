#ifndef SCENE_H
#define SCENE_H

#define SCENE_INTRO 0
#define SCENE_MENU 1
#define SCENE_BATTLE 2

/**
 * Struct for the function pointers of a Scene
*/
struct Scene
{
    void (*tick)();
    void (*init)();
    void (*complete)();
    void (*deconstruct)();
};

/**
 * The reference to the current scene
*/
extern int currentScene;

/**
 * Registers a Scene into the scene array (in scene.c)
 * @param index the index to set into
 * @param init the init function
 * @param tick the tick function
 * @param complete the complete function
 * @param deconstruct the function to deregister interrupts etc
*/
void registerScene(int index, void (*init)(), void(*tick)(), void(*complete)(), void(*deconstruct)());

/**
 * Switches scene, deconstructing the old one
 * @param scene the scene ID to switch to
*/
void switchScene(int scene);

/**
 * Initialises the current scene
 */
void scene_init();

/**
 * Runs on the current scene for every frame
 */
void scene_tick();

/**
 * Deconstructs the current scene
*/
void scene_deconstruct();

#endif