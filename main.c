#include "header/define.h"

int main(int argc, char* argv[])
{
    init_sys(argc, argv); // システム初期化

    // Wiiリモコンが接続状態の時はループ
    while (wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderPresent(renderer);
    }

    opening_process(); // システム開放

    return 0;
}