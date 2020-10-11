#include "header/define.h"
int interval  = 10; // マリオ描画の時間間隔
int min_flips = -1; // 1秒あたりの最小描画回数
// 時間間隔(flip_interval)あたりの最小描画回数を計算
Uint32 min_flips_callback(Uint32 flip_interval, void *param)
{
    int flips = *(int *)param;
    if (min_flips == -1 || flips < min_flips) {
        min_flips = flips;
    }
    // 描画回数を表示する
    fprintf(stderr, "Flips per sec: %d\n", flips);
    *(int *)param = 0;
    return flip_interval;
}

int main(int argc, char *argv[])
{
    init_sys(argc, argv); // システム初期化

    // Wiiリモコンが接続状態の時はループ
    while (wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    opening_process(); // システム開放

    return 0;
}