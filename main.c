#include "header/define.h"

char menu_str[5][10]       = { "SOLO", "MULTI", "SETTING", "EXIT" };
char menu_multi_str[5][10] = { "HOST", "CLIENT", "2 player", " 3 player", " 4 player" };

int menu_sel = 0; // メニューのセレクター

GameInfo gGame;     // ゲームの描画関係
Player gPlayer[4];  // プレイヤーの情報
int player_num = 1; // プレイヤーの数

SDL_Thread* wii_thread;      // wii_threadを用いる
SDL_Thread* wii_ir_thread;   // wii_ir_threadを用いる
SDL_Thread* keyboard_thread; // keyboard_threadを用いる
SDL_mutex* mtx;              // 相互排除（Mutex）
SDL_Surface* image_bg_1;     // 背景画像用のサーフェイス
SDL_Surface* image_menu_bg;  // メニュー画像用のサーフェイス
SDL_Texture* menu_texture;   // メニュー用のテクスチャ
SDL_Event event;             // SDLによるイベントを検知するための構造体

TTF_Font* font25;   // TrueTypeフォントデータを格納する構造体
TTF_Font* font50;   // TrueTypeフォントデータを格納する構造体
int iw, ih;         // 文字を描画する際に使用
SDL_Rect txtRect;   // 文字を描画する際に使用
SDL_Rect pasteRect; // 文字を描画する際に使用

SDL_Rect src_rect_bg = { 0, 0, WD_Width, WD_Height }; // 画像の切り取り範囲
SDL_Rect dst_rect_bg = { 0, 0 };                      // 描画位置
SDL_Rect imageRect;
SDL_Rect drawRect;

Uint32 rmask, gmask, bmask, amask; // サーフェイス作成時のマスクデータを格納する変数

wiimote_t wiimote = WIIMOTE_INIT; // Wiiリモコンの状態格納用

bool flag_loop = true;

int main(int argc, char* argv[])
{
    init_sys(argc, argv); // システム初期化

    // Wiiリモコンが接続状態の時はループ
    while (flag_loop) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        // モードによって条件分岐
        switch (gGame.mode) {
        // メニュー画面描画
        case MD_MENU:
            // メニュー画像を描画
            menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_menu_bg);
            SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
            imageRect = (SDL_Rect) { 0, 0, iw, ih };
            drawRect  = (SDL_Rect) { 0, 0, iw, ih };
            SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
            SDL_RenderClear(gGame.renderer);
            SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

            // セレクトボタンを配置
            for (int i = 0; i < 4; i++) {
                gGame.surface = TTF_RenderUTF8_Blended(font25, menu_str[i], (SDL_Color) { 0, 0, 0, 255 });
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 400, 270 + i * 50, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            }
            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 270 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "[MENU]", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 800, 445, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        // ソロプレイをプレイするかどうかを描画
        case MD_SOLO_WAIT:
            // メニュー画像を描画
            menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_menu_bg);
            SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
            imageRect = (SDL_Rect) { 0, 0, iw, ih };
            drawRect  = (SDL_Rect) { 0, 0, iw, ih };
            SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
            SDL_RenderClear(gGame.renderer);
            SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "Are you ready?", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 280, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクトボタンを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "OK", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 450, 340, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "CANCEL", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 450, 390, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 340 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "[SOLO PLAY]", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 725, 445, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        // ソロプレイ中
        case MD_SOLO_PLAYING:
            break;
        // マルチプレイ待機
        case MD_MULTI_WAIT:
            // メニュー画像を描画
            menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_menu_bg);
            SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
            imageRect = (SDL_Rect) { 0, 0, iw, ih };
            drawRect  = (SDL_Rect) { 0, 0, iw, ih };
            SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
            SDL_RenderClear(gGame.renderer);
            SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "Choose host or client.", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 250, 260, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクトボタンを配置
            for (int i = 0; i < 2; i++) {
                gGame.surface = TTF_RenderUTF8_Blended(font25, menu_multi_str[i], (SDL_Color) { 0, 0, 0, 255 });
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 400, 320 + i * 50, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            }

            gGame.surface = TTF_RenderUTF8_Blended(font25, "CANCEL", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 420, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 320 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "[MULTI PLAY]", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 700, 445, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        // マルチプレイ中
        case MD_MULTI_PLAYING:
            break;
        // 終了待機
        case MD_EXIT_WAIT:
            // メニュー画像を描画
            menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_menu_bg);
            SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
            imageRect = (SDL_Rect) { 0, 0, iw, ih };
            drawRect  = (SDL_Rect) { 0, 0, iw, ih };
            SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
            SDL_RenderClear(gGame.renderer);
            SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "Do you want to quit?", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 270, 270, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "OK", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 350, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            gGame.surface = TTF_RenderUTF8_Blended(font25, "CANCEL", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 400, 400, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

            // セレクターを描画
            gGame.surface = TTF_RenderUTF8_Blended(font25, "> ", (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 350 + menu_sel * 50, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            break;
        // 終了
        case MD_EXIT:
            Log("プログラムを終了します");
            flag_loop = false;
            break;
        default:
            break;
        }

        SDL_RenderPresent(gGame.renderer);
        SDL_Delay(10);
    }

    opening_process(); // システム開放
    printf("\n%s%s%s%s\n", COLOR_BOLD, COLOR_FG_LBLUE, "Bye.", COLOR_RESET);
    return 0;
}