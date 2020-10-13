#include "header/define.h"

int main(int argc, char *argv[])
{
    init_sys(argc, argv); // システム初期化

    // Wiiリモコンが接続状態の時はループ
    while (wiimote_is_open(&wiimote)) {
        SDL_RenderClear(gGame.renderer);

        SDL_FillRect(gGame.surface, NULL, 0x00000000);
        SDL_BlitSurface(image_bg, &src_rect_bg, gGame.surface, &dst_rect_bg);
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface); // 合成画像（サーフェイス）をテクスチャに転送
        SDL_RenderCopy(gGame.renderer, gGame.texture, NULL, NULL);                   // テクスチャをレンダラーにコピー
        SDL_RenderPresent(gGame.renderer);
    }

    opening_process(); // システム開放

    return 0;
}