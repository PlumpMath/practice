#include <stdio.h>

#include "platform.h"
#include "nanovg.h"

void
game_update_and_render(PlatformMemory *memory)
{
    nvgBeginPath(memory->vg);
    nvgRect(memory->vg, 100, 100, 100, 100);
    nvgFillColor(memory->vg, nvgRGBf(1,0,0));
    nvgFill(memory->vg);
}

Game game = {.update_and_render = game_update_and_render,
             .raw_memory_size = 1};
