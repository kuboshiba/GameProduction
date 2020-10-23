#include "header/define.h"

char menu_str[3][10] = { "SOLO", "MULTIPLE", "SETTING" };
int menu_mode        = 0;
int menu_sel         = 0;

int main(int argc, char *argv[])
{
    init_sys(argc, argv); // システム初期化

    // Wiiリモコンが接続状態の時はループ
    while (wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        switch (gGame.mode) {
        case MD_MENU:
            for (int i = 0; i < 3; i++) {
                gGame.surface = TTF_RenderUTF8_Blended(font, menu_str[i], (SDL_Color) { 255, 255, 255, 255 });
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 700, 100 + i * 50, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            }
            gGame.surface = TTF_RenderUTF8_Blended(font, "> ", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 650, 100 + menu_mode * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        case MD_SOLO_PLAY:
            gGame.surface = TTF_RenderUTF8_Blended(font, "[SOLO PLAY] Are you ready?", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 200, 100, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font, "OK", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 200, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font, "CANCEL", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 250, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font, "> ", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 200 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
        default:
            break;
        }

        SDL_RenderPresent(gGame.renderer);
        SDL_Delay(50);
    }

    opening_process(); // システム開放

    return 0;
}