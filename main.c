#include "header/define.h"

char menu_str[5][10]       = { "SOLO", "MULTIPLE", "SETTING" };
char menu_multi_str[5][10] = { "HOST", "CLIENT", "2 player", " 3 player", " 4 player" };

int menu_sel = 0; // メニューのセレクター

GameInfo gGame;     // ゲームの描画関係
Player gPlayer[4];  // プレイヤーの情報
int player_num = 1; // プレイヤーの数

SDL_Thread* wii_thread;      // wii_threadを用いる
SDL_Thread* keyboard_thread; // keyboard_threadを用いる
SDL_mutex* mtx;              // 相互排除（Mutex）
SDL_Surface* image_bg_1;     // 背景画像用のサーフェイス
SDL_Surface* image_menu_bg;  // メニュー画像陽のサーフェイス
SDL_Event event;             // SDLによるイベントを検知するための構造体

TTF_Font* font25;   // TrueTypeフォントデータを格納する構造体
TTF_Font* font50;   // TrueTypeフォントデータを格納する構造体
int iw, ih;         // 文字を描画する際に使用
SDL_Rect txtRect;   // 文字を描画する際に使用
SDL_Rect pasteRect; // 文字を描画する際に使用

SDL_Rect src_rect_bg = { 0, 0, WD_Width, WD_Height }; // 画像の切り取り範囲
SDL_Rect dst_rect_bg = { 0, 0 };                      // 描画位置

Uint32 rmask, gmask, bmask, amask; // サーフェイス作成時のマスクデータを格納する変数

wiimote_t wiimote = WIIMOTE_INIT; // Wiiリモコンの状態格納用

int main(int argc, char* argv[])
{
    init_sys(argc, argv); // システム初期化

    // Wiiリモコンが接続状態の時はループ
    while (wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        // モードによって条件分岐
        switch (gGame.mode) {
        // メニュー画面描画
        case MD_MENU:
            gGame.surface = TTF_RenderUTF8_Blended(font50, "NO TITLE", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 125, 200, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクトボタンを配置
            for (int i = 0; i < 3; i++) {
                gGame.surface = TTF_RenderUTF8_Blended(font25, menu_str[i], (SDL_Color) { 255, 255, 255, 255 });
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 700, 150 + i * 50, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            }
            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 650, 150 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        // ソロプレイをプレイするかどうかを描画
        case MD_SOLO_WAIT:
            gGame.surface = TTF_RenderUTF8_Blended(font25, "[SOLO PLAY] Are you ready?", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 200, 100, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクトボタンを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "OK", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 200, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "CANCEL", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 250, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 200 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        // ソロプレイ開始
        case MD_SOLO_PLAYING:
            break;
        case MD_MULTI_WAIT:
            gGame.surface = TTF_RenderUTF8_Blended(font25, "[MULTI PLAY] Choose host or client.", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 80, 100, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクトボタンを配置
            for (int i = 0; i < 2; i++) {
                gGame.surface = TTF_RenderUTF8_Blended(font25, menu_multi_str[i], (SDL_Color) { 255, 255, 255, 255 });
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 400, 200 + i * 50, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            }

            gGame.surface = TTF_RenderUTF8_Blended(font25, "CANCEL", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 300, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 255, 255, 255, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 200 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        case MD_MULTI_PLAYING:
            break;
        default:
            break;
        }

        SDL_RenderPresent(gGame.renderer);
        SDL_Delay(50);
    }

    opening_process(); // システム開放

    return 0;
}