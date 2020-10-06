#include "header/define.h"

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;

void Initialize()
{
    SDL_Init(SDL_INIT_VIDEO);

    window   = SDL_CreateWindow("No Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WD_Width, WD_Height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    while (true) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: // ウィンドウのXボタンが押されたとき開放処理を行い終了
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                exit(0);
            }
        }
        SDL_RenderPresent(renderer);
    }
}