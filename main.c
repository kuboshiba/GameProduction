#include "header/define.h"

SDL_Thread* wii_thread;        // wii_threadを用いる
SDL_Thread* keyboard_thread;   // keyboard_threadを用いる
SDL_Thread* wii_ir_thread;     // wii_ir_threadを用いる
SDL_mutex* mtx;                // 相互排除（Mutex）
SDL_Surface* image_bg_1;       // 背景画像用のサーフェイス
SDL_Surface* image_bg_2;       // 背景画像用のサーフェイス
SDL_Surface* image_bg_3;       // 背景画像用のサーフェイス
SDL_Surface* image_bg_4;       // 背景画像用のサーフェイス
SDL_Surface* image_menu_bg;    // メニュー画像用のサーフェイス
SDL_Surface* image_target[10]; // 的の画像用のサーフェイス
SDL_Texture* menu_texture;     // メニュー用のテクスチャ
SDL_Texture* solo_texture;     // ソロプレイ用のテクスチャ
SDL_Event event;               // SDLによるイベントを検知するための構造体
SDL_TimerID timer_id_1;        // min_flips_callback用のタイマー
SDL_TimerID timer_id_2;        // min_flips_callback用のタイマー

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
Target target[10];                 // 的の情報
Uint32 rmask, gmask, bmask, amask; // サーフェイス作成時のマスクデータを格納する変数

int iw, ih;              // 文字を描画する際に使用
int menu_sel       = 0;  // メニューのセレクター
int player_num     = 1;  // プレイヤーの数
int interval       = 40; // 描画の時間間隔
int min_flips      = -1; // 1秒あたりの最小描画回数
int count_down_val = 3;  // カウントダウン用の変数
int alpha_key_pos  = 0;  // キーボード入力のセレクタ

char menu_str[5][10]       = { "SOLO", "MULTI", "SETTING", "EXIT" };
char menu_multi_str[5][10] = { "HOST", "CLIENT", "2 player", " 3 player", " 4 player" };
char alpha[27][2]          = { "a", "b", "c", "d", "e", "f", "g", "h", "i",
    "j", "k", "l", "m", "n", "o", "p", "q", "r",
    "s", "t", "u", "v", "w", "x", "y", "z" };
char count_down_txt[100];

bool flag_loop    = true; // メインループのループフラグ
bool flag_playing = true; // プレイ用のループフラグ

Uint32 count_down(Uint32 interval, void* param);
Uint32 target_cnt(Uint32 interval, void* param);
Uint32 min_flips_callback(Uint32 flip_interval, void* param); // 時間間隔(flip_interval)あたりの最小描画回数を計算
void md_menu();
void md_solo_wait();
void md_solo_playing();
void md_multi_wait();
void md_exit_wait();

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
        filledCircleColor(gGame.renderer, pointer.x, pointer.y, 10, 0xff0000ff);

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
    alpha_key_pos = 27;
    player_num    = 1;                    // プレイヤーの数を取り敢えず１に初期化
    gGame.mode    = MD_PLAYER_NAME_INPUT; // モードをメニューに設定
    flag_playing  = true;

    // プレイヤー名入力用のループ
    while (flag_playing) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        // 画像を描画
        // menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_bg_2);
        // SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
        // imageRect = (SDL_Rect) { 0, 0, iw, ih };
        // drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        // SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
        // SDL_RenderClear(gGame.renderer);
        // SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

        // 背景に白のバックを追加
        // SDL_SetRenderDrawColor(gGame.renderer, 255, 255, 255, 255);
        // SDL_RenderFillRect(gGame.renderer, &(SDL_Rect) { 50, 50, 940, 440 });

        // プレイヤー名を入力してください　描画
        gGame.surface = TTF_RenderUTF8_Blended(font25, "Please input your name", (SDL_Color) { 255, 255, 255, 255 });
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
        SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 225, 50, iw, ih };
        SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

        // 入力したプレイヤー名のバックグラウンド
        SDL_SetRenderDrawColor(gGame.renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(gGame.renderer, &(SDL_Rect) { 225, 150, 550, 50 });

        gGame.surface = TTF_RenderUTF8_Blended(font25, gGame.name, (SDL_Color) { 255, 255, 255, 255 });
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
        SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 238, 160, iw, ih };
        SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

        // アルファベットの四角を描画
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 9; j++) {
                if (0 <= alpha_key_pos && alpha_key_pos <= 25 && alpha_key_pos == (j + 9 * i)) {
                    SDL_SetRenderDrawColor(gGame.renderer, 120, 120, 120, 255);
                    SDL_RenderFillRect(gGame.renderer, &(SDL_Rect) { 150 + j * 50, 250 + i * 50, 40, 40 });
                }
                SDL_SetRenderDrawColor(gGame.renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(gGame.renderer, &(SDL_Rect) { 150 + j * 50, 250 + i * 50, 40, 40 });
            }
        }

        // アルファベットを描画
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 9; j++) {
                gGame.surface = TTF_RenderUTF8_Blended(font25, alpha[j + i * 9], (SDL_Color) { 255, 255, 255, 255 });
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 158 + j * 50, 258 + i * 50, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
            }
        }

        // BS 26
        if (alpha_key_pos == 26) {
            SDL_SetRenderDrawColor(gGame.renderer, 120, 120, 120, 255);
            SDL_RenderFillRect(gGame.renderer, &(SDL_Rect) { 150, 400, 70, 50 });
        }
        SDL_SetRenderDrawColor(gGame.renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(gGame.renderer, &(SDL_Rect) { 150, 400, 70, 50 });

        gGame.surface = TTF_RenderUTF8_Blended(font25, "BS", (SDL_Color) { 255, 255, 255, 255 });
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
        SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 160, 410, iw, ih };
        SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

        // Enter 27
        if (alpha_key_pos == 27) {
            SDL_SetRenderDrawColor(gGame.renderer, 120, 120, 120, 255);
            SDL_RenderFillRect(gGame.renderer, &(SDL_Rect) { 245, 400, 145, 50 });
        }
        SDL_SetRenderDrawColor(gGame.renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(gGame.renderer, &(SDL_Rect) { 245, 400, 145, 50 });

        gGame.surface = TTF_RenderUTF8_Blended(font25, "Enter", (SDL_Color) { 255, 255, 255, 255 });
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
        SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 255, 410, iw, ih };
        SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

        // 登録されているプレイヤー名を表示
        gGame.surface = TTF_RenderUTF8_Blended(font25, "Player list", (SDL_Color) { 255, 255, 255, 255 });
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
        SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 650, 250, iw, ih };
        SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

        // ポインターをウィンドウに描画
        filledCircleColor(gGame.renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        // 描画
        SDL_RenderPresent(gGame.renderer);
        SDL_Delay(interval);
    }

    // カウントダウン用のタイマー起動
    timer_id_2 = SDL_AddTimer(1000, count_down, &count_down_val);

    flag_playing = true;
    // カウントダウン用のループ
    while (flag_playing) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        // 背景画像を描画
        menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_bg_1);
        SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        SDL_SetRenderDrawColor(gGame.renderer, 200, 200, 200, 255);
        SDL_RenderClear(gGame.renderer);
        SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

        // ポインターをウィンドウに描画
        filledCircleColor(gGame.renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        // カウントダウンを数値から文字列に変換
        sprintf(count_down_txt, "%d", count_down_val);

        // ０は文字列STARTを描画
        if (count_down_val <= 0) {
            sprintf(count_down_txt, "%s", "START");
            // カウントダウン表示
            gGame.surface = TTF_RenderUTF8_Blended(font50, count_down_txt, (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 200, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
        }
        // ３〜１ までのカウントダウン描画
        else {
            // カウントダウン表示
            gGame.surface = TTF_RenderUTF8_Blended(font50, count_down_txt, (SDL_Color) { 0, 0, 0, 255 });
            gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
            SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 450, 200, iw, ih };
            SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
        }

        // 描画
        SDL_RenderPresent(gGame.renderer);
        SDL_Delay(interval);

        // カウントダウンが -1 より小さければ break
        if (count_down_val <= -1) {
            flag_playing   = false;      // break
            count_down_val = 3;          // カウントダウンの変数を初期化
            SDL_RemoveTimer(timer_id_2); // タイマー解除
        }
    }

    flag_playing = true;
    gGame.mode   = MD_SOLO_PLAYING_1; // モードをメニューに設定
    srand(1);
    int target_num = rand() % 10;
    for (int i = 0; i < target_num; i++) {
        int type = rand() % 100;
        if (0 <= type && type < 30)
            type = 0;
        else if (30 <= type && type < 60)
            type = 1;
        else if (60 <= type && type < 85)
            type = 2;
        else if (85 <= type && type < 95)
            type = 3;
        else if (95 <= type && type < 100)
            type = 4;

        int x = 100 + rand() % 750;
        int y = 100 + rand() % 250;

        bool flag = true;

        for (int j = 0; j < TARGET_NUM_MAX; j++) {
            if (target[j].type != 5) {
                int a = target[j].x - x;
                int b = target[j].y - y;
                int c = sqrt(a * a + b * b);
                if (c <= 34)
                    flag = false;
            }
        }

        if (flag) {
            target[i].type = type;
            target[i].x    = x;
            target[i].y    = y;
            target[i].cnt  = 0;
        } else {
            target[i].type = 5;
            target[i].x    = 0;
            target[i].y    = 0;
            target[i].cnt  = 0;
        }
    }

    SDL_RemoveTimer(timer_id_2); // タイマー解除

    // 的の表示時間用のタイマーをセット
    timer_id_2 = SDL_AddTimer(1000, target_cnt, &target);

    // 実際のゲーム　ステージ１
    while (flag_playing) {
        SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gGame.renderer);

        // ステージ１の画像を描画
        menu_texture = SDL_CreateTextureFromSurface(gGame.renderer, image_bg_1);
        SDL_QueryTexture(menu_texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        SDL_RenderCopy(gGame.renderer, menu_texture, &imageRect, &drawRect);

        // 的を描画
        for (int i = 0; i < TARGET_NUM_MAX; i++) {
            if (target[i].type != 5) {
                gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, image_target[target[i].type]);
                SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
                imageRect = (SDL_Rect) { 0, 0, iw, ih };
                drawRect  = (SDL_Rect) { target[i].x, target[i].y, iw, ih };
                SDL_RenderCopy(gGame.renderer, gGame.texture, &imageRect, &drawRect);
            }
        }

        // スコアの表示
        char score_txt[100];
        sprintf(score_txt, "YOUR SCORE: %d", gGame.score);
        gGame.surface = TTF_RenderUTF8_Blended(font25, score_txt, (SDL_Color) { 255, 0, 0, 255 });
        gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
        SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 550, 100, iw, ih };
        SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

        // ポインターをウィンドウに描画
        filledCircleColor(gGame.renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(gGame.renderer);
        SDL_Delay(interval);
    }

    SDL_RemoveTimer(timer_id_2); // タイマー解除

    player_num = 1;       // プレイヤーの数を取り敢えず１に初期化
    gGame.mode = MD_MENU; // モードをメニューに設定
    menu_sel   = 0;       // セレクターを初期化
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

// カウントダウン用のタイマー
Uint32 count_down(Uint32 interval, void* param)
{
    count_down_val--;
    return interval;
}

// 的の表示時間用のタイマー
Uint32 target_cnt(Uint32 interval, void* param)
{
    for (int i = 0; i < TARGET_NUM_MAX; i++) {
        if (target[i].type != 5) {
            if (target[i].cnt == 3) {
                target[i].type = 5;
                target[i].cnt  = 0;
            } else
                target[i].cnt++;
        } else {
            if (target[i].cnt == 3) {
                int type = rand() % 100;
                if (0 <= type && type < 30)
                    type = 0;
                else if (30 <= type && type < 60)
                    type = 1;
                else if (60 <= type && type < 85)
                    type = 2;
                else if (85 <= type && type < 95)
                    type = 3;
                else if (95 <= type && type < 100)
                    type = 4;

                int x     = 100 + rand() % 750;
                int y     = 100 + rand() % 250;
                bool flag = true;
                for (int j = 0; j < TARGET_NUM_MAX; j++) {
                    if (target[j].type != 5) {
                        int a = target[j].x - x;
                        int b = target[j].y - y;
                        int c = sqrt(a * a + b * b);
                        if (c <= 50)
                            flag = false;
                    }
                }

                if (flag) {
                    target[i].type = type;
                    target[i].x    = x;
                    target[i].y    = y;
                    target[i].cnt  = 0;
                }
            } else {
                target[i].cnt++;
            }
        }
    }
    return interval;
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