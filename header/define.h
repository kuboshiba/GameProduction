#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

// ウインドウサイズ
enum {
    WD_Width  = 1000,
    WD_Height = 400
};

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Event event;

extern void Initialize();

#endif