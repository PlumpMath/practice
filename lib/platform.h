/*
  This is an sdl platform layer that is useful for practices that want to use a game engine setting.
  Sets up SDL2 with opengl 3.2 and nanovg, even sets up live code reloading.

  To use in a practice simply include platform.h, implement the interface and compile program as a shared
  library.
  
  Then invoke the platform with the practice name as an argument.
  $ ./practice 1989-10-28

  Any recompilations of the library will be updated live in the running program.
  See platform_test.c
 */

#ifndef _platform_h
#define _platform_h

#include <stdlib.h>
#include <stdbool.h>

#include "nanovg.h"

typedef struct {
    // Anything state that you want to persist accross live reloads should be stored in here. This memory is passed in
    // to your update function every frame, you can request how much you'd like allocated (via the interface you implement below)
    // and can then cast it to whatever struct you want for your game.
    uint64_t raw_memory_size;
    void* raw_memory;

    // Notification of the executable being reloaded.
    bool executable_reloaded;

    float dt; // seconds since last frame.
    uint64_t ticks; // ms since app began.

    // Width and height of the screen in normal pixels and double drawable area for retina.
    float display_width;
    float display_height;
    float drawable_width;
    float drawable_height;
    
    // Nanovg context is already set up because it is used for debug output. Only need to call drawing commands. All nanovg is rendered
    // after any of your own gl code.
    NVGcontext* vg;
    
} PlatformMemory;

// Interface you must implement in the practice file.
typedef void (*UpdateAndRenderFn)(PlatformMemory *memory);

typedef struct {
    UpdateAndRenderFn update_and_render;
    uint64_t raw_memory_size;
} Game;

extern Game game;

#endif
