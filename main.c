#include "header/define.h"

/* SDL2 */
SDL_Window* window;     // ウィンドウデータを格納する構造体
SDL_Renderer* renderer; // 2Dレンダリングコンテキスト（描画設定）を格納する構造体
SDL_Surface* surface;   // サーフェイス（メインメモリ上の描画データ）を格納する構造体
SDL_Texture* texture;   // テクスチャ（VRAM上の描画データ）を格納する構造体
SDL_Event event;        // SDLによるイベントを検知するための構造体

/* SDL2 スレッド関係 */
SDL_Thread* keyboard_thread;   // キーボード入力用のスレッド
SDL_Thread* wiimote_thread;    // Wiiリモコン入力用のスレッド
SDL_Thread* wiimote_ir_thread; // Wiiリモコンの赤外線センサの取得とポインター生成のスレッド

SDL_TimerID timer_id_countdown;        // カウントダウン用のタイマー
SDL_TimerID timer_id_transition_stage; // ステージ遷移用のタイマー
SDL_TimerID timer_id_target;           // 的の生成タイマー

/* MUSIC */
Mix_Music* bgm_menu; // BGM ファイルを読み込む構造体
int bgm_volume;      // BGM の音量

/* SDL2 画像関係の定義・変数 */
SDL_Surface* image_bg[IMAGE_BG_NUM];         // 背景画像
SDL_Surface* image_target[IMAGE_TARGET_NUM]; // 的の画像
SDL_Surface* image_menu;                     // メニュー画像
SDL_Surface* image_rect[10];                 // ゲーム中のステータスを表示する背景
SDL_Rect imageRect;                          // 画像の選択範囲
SDL_Rect drawRect;                           // 画像の描画位置
SDL_Rect txtRect;                            // 文字の選択範囲
SDL_Rect pasteRect;                          // 文字の描画範囲
POINT image_1_point = { 0, 0 };              // 現在画像の座標
POINT image_2_point = { 1000, 0 };           // 遷移する画像の座標
Uint32 rmask, gmask, bmask, amask;           // サーフェイス作成時のマスクデータを格納する変数
int iw, ih;                                  // テクスチャやサーフェイスの幅

/* Wiiリモコン関係 */
wiimote_t wiimote;                        // Wiiリモコンの状態格納用
SDL_Rect pointer      = { 0, 0, 15, 15 }; // Wiiリモコンの赤外線センサーの座標
SDL_Rect pointer_prev = { 0, 0, 15, 15 }; // Wiiリモコンの赤外線センサーの前の座標

/* その他 構造体 */
GameInfo gGame;                // ゲームで多用する共通変数は構造体 gGame で管理
PLAYER gPlayer;                // プレイヤーの情報
Font_PressStart2P fonts;       // PressStart2Pのフォント格納データ
Target target[TARGET_NUM_MAX]; // サーバーが持つ的の情報

/* その他 グローバル変数 */
int selecter       = 0;  // セレクター
int key_pos        = 0;  // プレイヤー名入力画面のセレクター
int count_down_val = 3;  // カウントダウン用の変数
int stage_pos      = 0;  // 何ステージかを格納
int interval       = 50; // SDL_Delayの遅延
char txt[100];
char alphabet[27][2] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" };
bool flag[MODE_NUM]  = { true }; // フラグ

/* main.c 関数 */
void mode_menu();                                  // メニュー画面を描画する関数
void mode_solo_ok_or_cancel();                     // ソロプレイをするかしないかを尋ねる関数
void mode_input_name();                            // プレイヤー名を入力する関数
void mode_solo_playing();                          // ソロプレイの処理
void mode_setting();                               // 設定を描画する画面
void count_down_draw(int);                         // カウントダウンの描画
void transition_stage(SDL_Surface*, SDL_Surface*); // 画面遷移関数
void create_target();                              // 的を生成する関数
void result_draw();                                // リザルトを描画する関数
Uint32 count_down(Uint32, void*);                  // カウントダウン処理
Uint32 timer_transition_stage(Uint32, void*);      // 画面遷移のアニメーション関数
Uint32 target_cnt(Uint32, void*);                  // タイマーで的を生成する関数

/* server 関係 */
CLIENT s_clients[MAX_NUM_CLIENTS]; // 構造体 CLIENT を構造体配列 s_clients
CONTAINER s_data;                  // 構造体 DATA を構造体変数 s_data で宣言
int s_num_clients;                 // クライアントの数を格納
int s_num_socks;                   // ソケットの数を格納
fd_set s_mask;

int server_main();
void setup_server(int, u_short);
int server_control_requests();
void server_send_data(int, void*, int);
int server_receive_data(int, void*, int);
void terminate_server(void);
void server_handle_error(char*);

/* client 関係 */
CLIENT c_clients[MAX_NUM_CLIENTS];
CONTAINER c_data; // 構造体 DATA を構造体変数 s_data で宣言
int c_sock;
int c_num_clients;
int c_myid;
int c_num_sock;
fd_set c_mask;

int client_main();
void setup_client(char*, u_short);
int client_control_requests();
int in_command(void);
int exe_command(void);
void client_send_data(void*, int);
int client_receive_data(void*, int);
void client_handle_error(char*);
void terminate_client();

int main(int argc, char* argv[])
{
    init_sys(argc, argv); // システム初期化

    /* ゲームがアクティブ状態であればループ */
    while (wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* ゲームのモードによって条件分岐 */
        switch (gGame.mode) {
        case MODE_MENU: // メニュー画面
            mode_menu();
            break;
        case MODE_SOLO_OK_OR_CANCEL:
            mode_solo_ok_or_cancel(); // ソロプレイをするかしないかを尋ねる
            break;
        case MODE_INPUT_NAME:
            mode_input_name(); // プレイヤー名の入力
            break;
        case MODE_SOLO_PLAYING:
            mode_solo_playing(); // ソロプレイ中
            break;
        case MODE_RESULT:
            result_draw(); // リザルト描画
            break;
        case MODE_SETTING: // 設定画面
            mode_setting();
            gGame.mode = MODE_MENU;
            break;
        default:
            break;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }

    opening_sys(); // システム開放
    printf("\n%s%s%s%s\n", COLOR_BOLD, COLOR_FG_LBLUE, "Bye.", COLOR_RESET);

    return 0;
}

/*******************************************************************
 * 関数名 : mode_menu
 * 　　型 : void
 * 　説明 : メニュー画面を描画する関数
 ******************************************************************/
void mode_menu()
{
    /* メニュー画像を描画 */
    texture = SDL_CreateTextureFromSurface(renderer, image_menu);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    imageRect = (SDL_Rect) { 0, 0, iw, ih };
    drawRect  = (SDL_Rect) { 0, 0, iw, ih };
    SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

    /* セレクトボタンを配置 */
    surface = TTF_RenderUTF8_Blended(fonts.size25, "SOLO", (SDL_Color) { 0, 0, 0, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 400, 270, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    surface = TTF_RenderUTF8_Blended(fonts.size25, "MULTI", (SDL_Color) { 0, 0, 0, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 400, 270 + 1 * 50, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    surface = TTF_RenderUTF8_Blended(fonts.size25, "SETTING", (SDL_Color) { 0, 0, 0, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 400, 270 + 2 * 50, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    surface = TTF_RenderUTF8_Blended(fonts.size25, "EXIT", (SDL_Color) { 0, 0, 0, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 400, 270 + 3 * 50, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    /* セレクターを描画 */
    surface = TTF_RenderUTF8_Blended(fonts.size25, "> ", (SDL_Color) { 0, 0, 0, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 350, 270 + selecter * 50, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    surface = TTF_RenderUTF8_Blended(fonts.size25, "[MENU]", (SDL_Color) { 0, 0, 0, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 800, 445, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    /* ポインターをウィンドウに描画 */
    filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);
}

/*******************************************************************
 * 関数名 : mode_solo_ok_or_cancel
 * 　　型 : void
 * 　説明 : ソロプレイをするかしないかを尋ねる関数
 ******************************************************************/
void mode_solo_ok_or_cancel()
{
    flag[MODE_SOLO_OK_OR_CANCEL] = true;
    while (flag[MODE_SOLO_OK_OR_CANCEL]) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* メニュー画像を描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_menu);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        surface = TTF_RenderUTF8_Blended(fonts.size25, "Are you ready?", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 350, 280, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* セレクトボタンを描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "OK", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 450, 340, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        surface = TTF_RenderUTF8_Blended(fonts.size25, "CANCEL", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 450, 390, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* セレクターを描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "> ", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 400, 340 + selecter * 50, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        surface = TTF_RenderUTF8_Blended(fonts.size25, "[SOLO PLAY]", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 725, 445, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* ポインターをウィンドウに描画 */
        filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }
}

/*******************************************************************
 * 関数名 : mode_input_name
 * 　　型 : void
 * 　説明 : プレイヤー名を入力する関数
 ******************************************************************/
void mode_input_name()
{
    flag[MODE_INPUT_NAME] = true;
    while (flag[MODE_INPUT_NAME] && wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* 「プレイヤー名を入力してください」を描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "Please input your name", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 158, 100, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* アルファベットの四角を描画 */
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 9; j++) {
                if (i == 2 && j == 8)
                    break;
                if (0 <= key_pos && key_pos <= 25 && key_pos == (j + 9 * i)) {
                    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
                    SDL_RenderFillRect(renderer, &(SDL_Rect) { 150 + j * 50, 250 + i * 50, 40, 40 });
                }
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &(SDL_Rect) { 150 + j * 50, 250 + i * 50, 40, 40 });
            }
        }

        /* アルファベットを描画 */
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 9; j++) {
                surface = TTF_RenderUTF8_Blended(fonts.size25, alphabet[j + i * 9], (SDL_Color) { 255, 255, 255, 255 });
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
                txtRect   = (SDL_Rect) { 0, 0, iw, ih };
                pasteRect = (SDL_Rect) { 158 + j * 50, 258 + i * 50, iw, ih };
                SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);
            }
        }

        /* ボタンの説明の描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size20, "Press A: Input", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 650, 250, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        surface = TTF_RenderUTF8_Blended(fonts.size20, "Press B: Delete ", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 650, 300, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        surface = TTF_RenderUTF8_Blended(fonts.size20, "Press 1: Play ", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 650, 350, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* プレイヤー名の描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, gPlayer.name, (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 158, 180, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &(SDL_Rect) { 150, 170, 450, 50 });

        /* ポインターをウィンドウに描画 */
        filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }
}

/*******************************************************************
 * 関数名 : mode_solo_playing
 * 　　型 : void
 * 　説明 : ソロプレイ中に実行する関数
 ******************************************************************/
void mode_solo_playing()
{
    stage_pos     = 0; // ステージを 0 に初期化
    gPlayer.score = 0; // プレイヤーのスコアを初期化
    /* ステージ１のカウントダウンを描画する */
    gGame.mode = MODE_COUNTDOWN;    // モードをカウントダウンに
    count_down_draw(stage_pos);     // カウントダウン開始
    gGame.mode = MODE_SOLO_PLAYING; // モードをソロプレイ中に

    /* 時間の記録（スタート） */
    double startTime, endTime;
    int totalTime = 0;
    startTime     = clock() / CLOCKS_PER_SEC;

    /* 的の生成と的の表示時間用のタイマーをセット */
    create_target();
    timer_id_target = SDL_AddTimer(1000, target_cnt, &target);

    /* ステージ１〜４までループ */
    flag[MODE_SOLO_PLAYING] = true;
    while (flag[MODE_SOLO_PLAYING] && wiimote_is_open(&wiimote)) {
        /* 経過時間を算出 */
        endTime   = clock() / CLOCKS_PER_SEC;
        totalTime = (int)(endTime - startTime);

        /* 経過時間が STAGE_TIME になるとステージ遷移 */
        if (totalTime >= STAGE_TIME + 1) {
            SDL_RemoveTimer(timer_id_target);
            /* ラストステージじゃなかったらステージを遷移 */
            if (stage_pos != 3) {
                transition_stage(image_bg[stage_pos], image_bg[stage_pos + 1]); // ステージ遷移アニメーション
                stage_pos++;                                                    // ステージをインクリメント
                gGame.mode = MODE_COUNTDOWN;                                    // モードをカウントダウンに
                count_down_draw(stage_pos);                                     // カウントダウン開始
                gGame.mode = MODE_SOLO_PLAYING;                                 // モードをソロプレイ中に

                /* 時間の記録（スタート） */
                totalTime = 0;
                startTime = clock() / CLOCKS_PER_SEC;

                /* 経過時間を算出 */
                endTime   = clock() / CLOCKS_PER_SEC;
                totalTime = (int)(endTime - startTime);

                /* 的の生成と的の表示時間用のタイマーをセット */
                create_target();
                timer_id_target = SDL_AddTimer(1000, target_cnt, &target);
            }
            /* ラストステージの場合 */
            else {
                /* ソロプレイ中のループから抜ける */
                flag[MODE_SOLO_PLAYING] = false;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* 背景画像を描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_bg[stage_pos]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* 右上に透過画像を描画　この上にステータスを描く */
        texture = SDL_CreateTextureFromSurface(renderer, image_rect[0]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, 400, 100 };
        drawRect  = (SDL_Rect) { 600, 0, 400, 100 };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* ステージ番号を描画 */
        sprintf(txt, "%s%d%s", "STAGE ", stage_pos + 1, " / 4");
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 10, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* 残り秒数を描画 */
        sprintf(txt, "%s%d%s%d%s", "TIME  ", totalTime, " / ", STAGE_TIME, " sec");
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 40, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* スコアを描画 */
        sprintf(txt, "%s%d", "SCORE ", gPlayer.score);
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 70, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* 的を描画 */
        for (int i = 0; i < TARGET_NUM_MAX; i++) {
            if (s_data.target[i].type != 5) {
                texture = SDL_CreateTextureFromSurface(renderer, image_target[s_data.target[i].type]);
                SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
                imageRect = (SDL_Rect) { 0, 0, iw, ih };
                drawRect  = (SDL_Rect) { s_data.target[i].x, s_data.target[i].y, iw, ih };
                SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);
            }
        }

        /* ポインターをウィンドウに描画 */
        filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }

    gGame.mode = MODE_RESULT;
}

/*******************************************************************
 * 関数名 : transition_stage
 * 　　型 : void
 * 　引数 : int stage_pos (現在のステージ番号)
 * 　説明 : ステージ遷移のアニメーションを実行する関数
 ******************************************************************/
void transition_stage(SDL_Surface* image_1, SDL_Surface* image_2)
{
    int mode_buf = gGame.mode; // 現在のモードを記憶

    timer_id_transition_stage = SDL_AddTimer(4, timer_transition_stage, NULL);

    gGame.mode            = MODE_TRANSITION;
    flag[MODE_TRANSITION] = true;
    while (flag[MODE_TRANSITION] && wiimote_is_open(&wiimote)) {
        /* 画像２が x座標０で描画されたら break */
        if (image_2_point.x <= 0) {
            image_1_point.x = 0;
            image_2_point.x = 1000;

            flag[MODE_TRANSITION] = false;
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* 背景画像１を描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_1);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { image_1_point.x, image_1_point.y, iw, ih };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* 背景画像２を描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_2);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { image_2_point.x, image_2_point.y, iw, ih };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* 右上に透過画像を描画　この上にステータスを描く */
        texture = SDL_CreateTextureFromSurface(renderer, image_rect[0]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, 400, 100 };
        drawRect  = (SDL_Rect) { 600, 0, 400, 100 };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* ステージ番号を描画 */
        sprintf(txt, "%s%d%s%d%s", "STAGE ", stage_pos + 1, " >>> ", stage_pos + 2, " / 4");
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 10, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* 残り秒数を描画 */
        sprintf(txt, "%s%d%s%d%s", "TIME  ", 0, " / ", STAGE_TIME, " sec");
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 40, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* スコアを描画 */
        sprintf(txt, "%s%d", "SCORE ", gPlayer.score);
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 70, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* ポインターをウィンドウに描画 */
        filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }

    SDL_RemoveTimer(timer_id_transition_stage);

    gGame.mode = mode_buf;
}

/*******************************************************************
 * 関数名 : timer_transition_stage
 * 　　型 : Uint32
 * 　引数 : Uint32 interval (タイマーのms) void* param (パラメータ)
 * 　説明 : ステージ遷移のアニメーションをするタイマー関数
 ******************************************************************/
Uint32 timer_transition_stage(Uint32 interval, void* param)
{
    image_1_point.x--;
    image_2_point.x--;

    return interval;
}

/*******************************************************************
 * 関数名 : count_down_draw
 * 　　型 : void
 * 　引数 : stage_pos (何ステージか)
 * 　説明 : カウントダウン用のタイマー
 ******************************************************************/
void count_down_draw(int stage_pos)
{
    /* カウントダウン用のタイマー起動 */
    timer_id_countdown   = SDL_AddTimer(1000, count_down, &count_down_val);
    flag[MODE_COUNTDOWN] = true;
    while (flag[MODE_COUNTDOWN] && wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* 背景画像を描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_bg[stage_pos]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* カウントダウンを数値から文字列に変換 */
        sprintf(txt, "%d", count_down_val);

        /* ０は文字列STARTを描画 */
        if (count_down_val <= 0) {
            sprintf(txt, "%s", "START");
            /* カウントダウン表示 */
            surface = TTF_RenderUTF8_Blended(fonts.size50, txt, (SDL_Color) { 255, 0, 0, 255 });
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 350, 250, iw, ih };
            SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);
        }
        /* ３〜１ までのカウントダウン描画 */
        else {
            /* カウントダウン表示 */
            surface = TTF_RenderUTF8_Blended(fonts.size50, txt, (SDL_Color) { 255, 0, 0, 255 });
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
            txtRect   = (SDL_Rect) { 0, 0, iw, ih };
            pasteRect = (SDL_Rect) { 450, 250, iw, ih };
            SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);
        }

        /* 右上に透過画像を描画　この上にステータスを描く */
        texture = SDL_CreateTextureFromSurface(renderer, image_rect[0]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, 400, 100 };
        drawRect  = (SDL_Rect) { 600, 0, 400, 100 };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* ステージ番号を描画 */
        sprintf(txt, "%s%d%s", "STAGE ", stage_pos + 1, " / 4");
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 10, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* 残り秒数を描画 */
        sprintf(txt, "%s%d%s%d%s", "TIME  ", 0, " / ", STAGE_TIME, " sec");
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 40, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* スコアを描画 */
        sprintf(txt, "%s%d", "SCORE ", gPlayer.score);
        surface = TTF_RenderUTF8_Blended(fonts.size20, txt, (SDL_Color) { 30, 30, 30, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 610, 70, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* ポインターをウィンドウに描画 */
        filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);

        /* カウントダウンが -1 より小さければ break */
        if (count_down_val <= -1) {
            flag[MODE_COUNTDOWN] = false;        // break
            count_down_val       = 3;            // カウントダウンの変数を初期化
            SDL_RemoveTimer(timer_id_countdown); // タイマー解除
        }
    }
}

/*******************************************************************
 * 関数名 : count_down
 * 　　型 : Uint32
 * 　引数 : Uint32 interval (タイマーのms) void* param (パラメータ)
 * 　説明 : カウントダウン用のタイマー
 ******************************************************************/
Uint32 count_down(Uint32 interval, void* param)
{
    count_down_val--;
    return interval;
}

/*******************************************************************
 * 関数名 : create_target
 * 　　型 : void
 * 　説明 : 的を生成する関数
 ******************************************************************/
void create_target()
{
    /* 座標関係初期化 */
    for (int i = 0; i < TARGET_NUM_MAX; i++) {
        target[i].type        = 5;
        target[i].x           = 0;
        target[i].y           = 0;
        target[i].cnt         = 0;
        s_data.target[i].type = 5;
        s_data.target[i].x    = 0;
        s_data.target[i].y    = 0;
        s_data.target[i].cnt  = 0;
        c_data.target[i].type = 5;
        c_data.target[i].x    = 0;
        c_data.target[i].y    = 0;
        c_data.target[i].cnt  = 0;
    }

    /* 的を生成する */
    srand(((unsigned)time(NULL)));
    for (int i = 0; i < TARGET_NUM_MAX; i++) {
        int probability = rand() % 100;           // ０〜１００の間で乱数生成
        int type        = 5;                      // タイプを初期化
        if (0 <= probability && probability < 30) // target 100
            type = 0;
        else if (30 <= probability && probability < 60) // target 200
            type = 1;
        else if (60 <= probability && probability < 75) // target 500
            type = 2;
        else if (75 <= probability && probability < 85) // target 1000
            type = 3;
        else if (85 <= probability && probability < 90) // target 2000
            type = 4;
        else if (90 <= probability && probability < 100) // hide target
            type = 5;

        /* 的を表示する場合 */
        if (type != 5) {
            int x = 50 + rand() % 900;  // x座標生成
            int y = 100 + rand() % 300; // y座標生成

            bool generate = true; // 的を生成出来るかどうか
            /* 生成した座標と既存の的の座標から当たり判定を行う */
            for (int j = 0; j < TARGET_NUM_MAX; j++) {
                /* 表示している的であれば */
                if (s_data.target[j].type != 5) {
                    int a = s_data.target[j].x - x;
                    int b = s_data.target[j].y - y;
                    int c = sqrt(a * a + b * b);
                    if (c <= 50) { // 当たっている
                        generate = false;
                        break;
                    }
                }
            }

            /* 的を生成できる場合 */
            if (generate) {
                s_data.target[i].type = type;
                s_data.target[i].x    = x;
                s_data.target[i].y    = y;
                s_data.target[i].cnt  = 0;
            }
            /* 的を生成出来なかった場合 */
            else {
                s_data.target[i].type = 5;
                s_data.target[i].x    = 0;
                s_data.target[i].y    = 0;
                s_data.target[i].cnt  = 0;
            }
        }
    }
}

/*******************************************************************
 * 関数名 : target_cnt
 * 　　型 : Uint32
 * 　引数 : Uint32 interval (タイマーのms) void* param (パラメータ)
 * 　説明 : 的の生成タイマー関数
 ******************************************************************/
Uint32 target_cnt(Uint32 interval, void* param)
{
    /* 的を全てチェック */
    for (int i = 0; i < TARGET_NUM_MAX; i++) {
        /* 表示している的であれば */
        if (s_data.target[i].type != 5) {
            /* カウンターが既定値に達していれば的を消す */
            if (s_data.target[i].cnt == 3) {
                s_data.target[i].type = 5;
                s_data.target[i].x    = 0;
                s_data.target[i].y    = 0;
                s_data.target[i].cnt  = 0;
            }
            /* 的のカウンターをインクリメント */
            else
                s_data.target[i].cnt++;
        }
        /* 表示していない的であれば */
        else if (s_data.target[i].type == 5) {
            if (s_data.target[i].cnt == 2) {
                int probability = rand() % 100;           // ０〜１００の間で乱数生成
                int type        = 5;                      // タイプを初期化
                if (0 <= probability && probability < 30) // target 100
                    type = 0;
                else if (30 <= probability && probability < 60) // target 200
                    type = 1;
                else if (60 <= probability && probability < 75) // target 500
                    type = 2;
                else if (75 <= probability && probability < 85) // target 1000
                    type = 3;
                else if (85 <= probability && probability < 90) // target 2000
                    type = 4;
                else if (90 <= probability && probability < 100) // hide target
                    type = 5;

                /* 的を表示する場合 */
                if (type != 5) {
                    int x = 50 + rand() % 900;  // x座標生成
                    int y = 100 + rand() % 300; // y座標生成

                    bool generate = true; // 的を生成出来るかどうか
                    /* 生成した座標と既存の的の座標から当たり判定を行う */
                    for (int j = 0; j < TARGET_NUM_MAX; j++) {
                        /* 表示している的であれば */
                        if (s_data.target[j].type != 5) {
                            int a = s_data.target[j].x - x;
                            int b = s_data.target[j].y - y;
                            int c = sqrt(a * a + b * b);
                            if (c <= 50) { // 当たっている
                                generate = false;
                                break;
                            }
                        }
                    }

                    /* 的を生成できる場合 */
                    if (generate) {
                        s_data.target[i].type = type;
                        s_data.target[i].x    = x;
                        s_data.target[i].y    = y;
                        s_data.target[i].cnt  = 0;
                    }
                    /* 的を生成出来なかった場合 */
                    else {
                        s_data.target[i].type = 5;
                        s_data.target[i].x    = 0;
                        s_data.target[i].y    = 0;
                        s_data.target[i].cnt  = 0;
                    }
                }
            }
            s_data.target[i].cnt++; // 的のカウンターをインクリメント
        }
    }
    return interval;
}

/*******************************************************************
 * 関数名 : result_draw
 * 　　型 : void
 * 　説明 : リザルトを描画する関数
 ******************************************************************/
void result_draw()
{
    gGame.mode        = MODE_RESULT;
    flag[MODE_RESULT] = true;
    while (flag[MODE_RESULT] && wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        surface = TTF_RenderUTF8_Blended(fonts.size50, "RESULT", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 350, 100, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* 名前を描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, gPlayer.name, (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 400, 200, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* スコアを描画 */
        sprintf(txt, "%s%d", "SCORE ", gPlayer.score);
        surface = TTF_RenderUTF8_Blended(fonts.size25, txt, (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 380, 280, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* OKボタンを描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "OK", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 300, 370, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* REPLAYボタンを描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "REPLAY", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 600, 370, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* セレクターを描画 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, ">", (SDL_Color) { 255, 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 250 + selecter * 300, 370, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* ポインターをウィンドウに描画 */
        filledCircleColor(renderer, pointer.x, pointer.y, 10, 0xff0000ff);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }
}

/*******************************************************************
 * 関数名 : mode_setting
 * 　　型 : void
 * 　説明 : 設定画面を描画する画面
 ******************************************************************/
void mode_setting()
{
    flag[MODE_SETTING] = true;
    while (flag[MODE_SETTING]) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* メニュー画像を描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_bg[0]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 0, 0, iw, ih };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        /* 薄いグレーを描画 */
        texture = SDL_CreateTextureFromSurface(renderer, image_rect[1]);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        imageRect = (SDL_Rect) { 0, 0, iw, ih };
        drawRect  = (SDL_Rect) { 100, 50, iw, ih };
        SDL_RenderCopy(renderer, texture, &imageRect, &drawRect);

        surface = TTF_RenderUTF8_Blended(fonts.size25, ">", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 200, 200 + selecter * 80, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        /* BGM の設定 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "BGM", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 250, 200, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect) { 410, 200, 400, 20 });

        filledCircleColor(renderer, 410, 210, 10, 0xff0000ff);

        /* SE の設定 */
        surface = TTF_RenderUTF8_Blended(fonts.size25, "SE", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 250, 280, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect) { 410, 280, 400, 20 });

        filledCircleColor(renderer, 410, 290, 10, 0xff0000ff);

        surface = TTF_RenderUTF8_Blended(fonts.size25, "Press 1: OK", (SDL_Color) { 0, 0, 0, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
        txtRect   = (SDL_Rect) { 0, 0, iw, ih };
        pasteRect = (SDL_Rect) { 350, 400, iw, ih };
        SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(interval);
    }
}
