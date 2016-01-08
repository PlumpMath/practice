/* 
   Platform layer for practices. Runs and reloads practice sessions that conform to the platform.h interface.
   Practices are throwaway code so this platform layer can evolve to support more features or change in ways
   that breaks previous practice sessions and that is just fine.

   @TODO:
   nanovg update loop is controlled from here so I can draw some debug ui for stuff like framerate and maybe some logging functionality.
 */
#include <SDL2/SDL.h>

#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <OpenGL/gl3.h>
#include "platform.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#include <sys/stat.h>
#include <dlfcn.h>


typedef struct {
    void* handle;
    ino_t id;
    Game* game;
} CurrentGame;

bool
current_game_reload(CurrentGame* current_game, const char* library)
{
    bool game_reloaded = false;
    
    struct stat attr;
    if ((stat(library, &attr) == 0) && (current_game->id != attr.st_ino)) {
        fprintf(stderr, "New Library to load.\n");
        
        if (current_game->handle) {
            dlclose(current_game->handle);
        }
        void* handle = dlopen(library, RTLD_NOW);

        if (handle) {
            current_game->handle = handle;
            current_game->id = attr.st_ino;

            Game* game = (Game*)dlsym(handle, "game");
            if (game != NULL) {
                current_game->game = game;
                fprintf(stderr, "Reloaded Game Library\n");
                game_reloaded = true;
                
            } else {
                fprintf(stderr, "[error] Error loading api symbol.\n");
                dlclose(handle);
                current_game->handle = NULL;
                current_game->id = 0;
            }
            
        } else {
            fprintf(stderr, "Error loading game library.\n");
            current_game->handle = NULL;
            current_game->id = 0;
        }
    }

    return game_reloaded;
}

int
main(int argc, char* argv[])
{
    // @TODO: pass in the name of the practice session to load up.
    if (argc != 2) {
        fprintf(stderr, "Error: you must pass in the name of the practice, (eg 1234-56-89)\n");
        return 1;
    }

    const char* practice = argv[1];
    char library_name[256] = {'\0'};
    snprintf(library_name, 256, "%s.dylib", practice);
    const char* library = library_name;

    CurrentGame current_game = {.handle = NULL,
                                .id = 0,
                                .game = NULL};

    current_game_reload(&current_game, library);

    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Error initializing SDL: %s\ns", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window *window = SDL_CreateWindow(practice,
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1920,
                                          1080,
                                          SDL_WINDOW_OPENGL |
                                          SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
    }

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    fprintf(stderr, "Drawable Resolution: %d x %d\n", width, height);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        fprintf(stderr, "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    // Setup nvg
    NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
        fprintf(stderr, "Could not init nanovg.\n");
    }

    uint64_t needed_memory = current_game.game->raw_memory_size;

    PlatformMemory memory = {.raw_memory_size = needed_memory,
                             .raw_memory = calloc(1, needed_memory),
                             .vg = vg,
    };


    uint32_t update_time = 0;
    uint32_t frame_time = 0;

    uint64_t last_counter = SDL_GetPerformanceCounter();
    uint32_t last_start_time = SDL_GetTicks();

    bool running = true;
    while (running) {
        uint32_t start_time = SDL_GetTicks();
        frame_time = start_time - last_start_time;
        float dt = (float)frame_time / 1000.0;
        last_start_time = start_time;

        int w, h;
	SDL_GetWindowSize(window, &w, &h);

        int draw_w, draw_h;
        SDL_GL_GetDrawableSize(window, &draw_w, &draw_h);

        memory.executable_reloaded = current_game_reload(&current_game, library);

        memory.ticks = start_time;
        memory.dt = dt;
        memory.display_width = w;
        memory.display_height = h;
        memory.drawable_width = draw_w;
        memory.drawable_height = draw_h;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                running = false;
                break;
            };
        }

        if (false == running) {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        nvgBeginFrame(memory.vg, memory.display_height, memory.display_width, memory.drawable_height/memory.display_height);

        current_game.game->update_and_render(&memory);

        nvgEndFrame(memory.vg);

        update_time = SDL_GetTicks() - start_time;

        float time_till_vsync = 1000.0/60.0 - (SDL_GetTicks() - start_time);
        if (time_till_vsync > 3) {
            SDL_Delay(time_till_vsync - 2);
        }

        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        last_counter = end_counter;
    }

    fprintf(stderr, "Closing\n");
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
