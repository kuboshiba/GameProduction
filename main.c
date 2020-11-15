#include "header/define.h"

SDL_Thread* wii_thread;      // wii_threadを用いる
SDL_Thread* keyboard_thread; // keyboard_threadを用いる
SDL_mutex* mtx;              // 相互排除（Mutex）
SDL_Surface* image_bg_1;     // 背景画像用のサーフェイス
SDL_Surface* image_bg_2;     // 背景画像用のサーフェイス
SDL_Surface* image_bg_3;     // 背景画像用のサーフェイス
SDL_Surface* image_menu_bg;  // メニュー画像用のサーフェイス
SDL_Texture* menu_texture;   // メニュー用のテクスチャ
SDL_Event event;             // SDLによるイベントを検知するための構造体
SDL_TimerID timer_id_1;      // min_flips_callback用のタイマー
SDL_TimerID timer_id_2;      // min_flips_callback用のタイマー

TTF_Font* font25; // TrueTypeフォントデータを格納する構造体
TTF_Font* font50; // TrueTypeフォントデータを格納する構造体

SDL_Rect pointer      = { 0, 0, 15, 15 };              // ポインター
SDL_Rect pointer_prev = { 0, 0, 15, 15 };              // 前回のポインター
SDL_Rect src_rect_bg  = { 0, 0, WD_Width, WD_Height }; // 画像の切り取り範囲
SDL_Rect dst_rect_bg  = { 0, 0 };                      // 描画位置
SDL_Rect txtRect;                                      // 文字を描画する際に使用
SDL_Rect pasteRect;                                    // 文字を描画する際に使用
SDL_Rect imageRect;
SDL_Rect drawRect;

wiimote_t wiimote = WIIMOTE_INIT; // Wiiリモコンの状態格納用

GameInfo gGame;                    // ゲームの描画関係
Player gPlayer[4];                 // プレイヤーの情報
Uint32 rmask, gmask, bmask, amask; // サーフェイス作成時のマスクデータを格納する変数

int menu_sel = 0;   // メニューのセレクター
int player_num;     // プレイヤーの数
int iw, ih;         // 文字を描画する際に使用
int interval  = 40; // 描画の時間間隔
int min_flips = -1; // 1秒あたりの最小描画回数

char menu_str[5][10]       = { "SOLO", "MULTI", "SETTING", "EXIT" };
char menu_multi_str[5][10] = { "HOST", "CLIENT", "2 player", " 3 player", " 4 player" };

bool flag_loop = true; // メインループのループフラグ

void md_menu();
void md_solo_wait();
void md_solo_playing();
void md_multi_wait();
void md_exit_wait();
Uint32 min_flips_callback(Uint32 flip_interval, void* param); // 時間間隔(flip_interval)あたりの最小描画回数を計算

int main(int argc, char* argv[])
{
    init_sys(argc, argv); // システム初期化

    int flips  = 0;                                              // 1秒あたりの描画回数
    timer_id_1 = SDL_AddTimer(1000, min_flips_callback, &flips); // 1秒あたりの最小描画回数を計算

    // Wiiリモコンが接続状態の時はループ
    while (flag_loop) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        // モードによって条件分岐
        switch (gGame.mode) {
        // メニュー画面描画
        case MD_MENU:
            md_menu();
            break;
        // ソロプレイをプレイするかどうかを描画
        case MD_SOLO_WAIT:
            md_solo_wait();
            break;
        // ソロプレイ中
        case MD_SOLO_PLAYING:
            md_solo_playing();
            break;
        // マルチプレイ待機
        case MD_MULTI_WAIT:
            md_multi_wait();
            break;
        // マルチプレイ中
        case MD_MULTI_PLAYING:
            break;
        // 終了待機
        case MD_EXIT_WAIT:
            md_exit_wait();
            break;
        // 終了
        case MD_EXIT:
            Log("プログラムを終了します");
            flag_loop = false;
            break;
        default:
            break;
        }

        // ポインターをウィンドウに描画
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(gGame.renderer, &pointer);

        SDL_RenderPresent(gGame.renderer);

        // flips += 1; // 表示回数
        SDL_Delay(interval);
    }

    opening_process(); // システム開放
    printf("\n%s%s%s%s\n", COLOR_BOLD, COLOR_FG_LBLUE, "Bye.", COLOR_RESET);
    return 0;
}

void md_solo_playing()
{
    // メニュー画像を描画
    menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_bg_2);
    SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
    imageRect = (SDL_Rect) { 0, 0, iw, ih };
    drawRect  = (SDL_Rect) { 0, 0, iw, ih };
    SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
    SDL_RenderClear(gGame.renderer);
    SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);
}

void md_menu()
{
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
}

void md_solo_wait()
{
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
}

void md_multi_wait()
{
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
}

void md_exit_wait()
{
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
}

// 時間間隔(flip_interval)あたりの最小描画回数を計算
Uint32 min_flips_callback(Uint32 flip_interval, void* param)
{
    int flips = *(int*)param;
    if (min_flips == -1 || flips < min_flips) {
        min_flips = flips;
    }
    // 描画回数を表示する
    // fprintf(stderr, "Flips per sec: %d\n", flips);
    *(int*)param = 0;
    return flip_interval;
}