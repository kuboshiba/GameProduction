#include "header/define.h"

GameInfo gGame;    // ゲームの描画関係
Player gPlayer[4]; // プレイヤーの情報

SDL_Thread* wii_thread;      // wii_threadを用いる
SDL_Thread* keyboard_thread; // keyboard_threadを用いる
SDL_mutex* mtx;              // 相互排除（Mutex）
SDL_Surface* image_bg;       // 背景画像用のサーフェイス
SDL_Event event;             // SDLによるイベントを検知するための構造体

SDL_Rect src_rect_bg = { 0, 0, WD_Width, WD_Height }; // 画像の切り取り範囲
SDL_Rect dst_rect_bg = { 0, 0 };                      // 描画位置

Uint32 rmask, gmask, bmask, amask; // サーフェイス作成時のマスクデータを格納する変数

wiimote_t wiimote = WIIMOTE_INIT; // Wiiリモコンの状態格納用

// SDLやWiiリモコンを初期化する関数
void init_sys(int argc, char* argv[])
{
    // SDL初期化
    SDL_Init(SDL_INIT_VIDEO);
    // SDL_IMG初期化
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    image_bg = IMG_Load("./image/bg1.png");

    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        Error("SDLの初期化に失敗しました");
        exit(-1);
    }

    // ウィンドウ生成
    if ((gGame.window = SDL_CreateWindow("No Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WD_Width, WD_Height, 0)) == NULL) {
        Error("ウィンドウの生成に失敗しました");
        exit(-1);
    }

    // レンダリングコンテキスト（RC）作成
    if ((gGame.renderer = SDL_CreateRenderer(gGame.window, -1, 0)) == NULL) {
        Error("レンダラーの生成に失敗しました");
        exit(-1);
    }

    // 初期画面
    SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255);                                                  // 生成したRCに描画色として青を設定
    SDL_RenderClear(gGame.renderer);                                                                       // 設定色でRCをクリア
    stringColor(gGame.renderer, 0, 0, "Press buttons 1 and 2 on the wiimote now to connect.", 0xffffffff); // 文字列を描画
    SDL_RenderPresent(gGame.renderer);

    // Wiiリモコン処理
    if (argc < 2) { // Wiiリモコン識別情報がコマンド引数で与えられなければ
        Error("WiiリモコンのMACアドレスを引数に指定してください");
        exit(1);
    }

    // Wiiリモコンの接続（１つのみ）
    // コマンド引数に指定したWiiリモコン識別情報を渡して接続
    if (wiimote_connect(&wiimote, argv[1]) < 0) {
        Error("Wiiリモコンの接続に失敗しました");
        exit(1);
    }

    wiimote.led.one  = 1; // WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
    wiimote.mode.acc = 1; // センサからのデータを受け付けるモードに変更

    wiimote_update(&wiimote); // Wiiリモコンの状態更新

    // 相互排除（Mutex）あり
    mtx = SDL_CreateMutex(); // 相互排除（Mutex）を用いる

    // スレッドを作成・実行
    wii_thread      = SDL_CreateThread(wii_func, "wii_thread", mtx);
    keyboard_thread = SDL_CreateThread(keyboard_func, "keyboard_thread", mtx);

// マスクを設定
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    // 合成画像作成用サーフェイスを作成
    gGame.surface = SDL_CreateRGBSurface(0, WD_Width, WD_Height, 32, rmask, gmask, bmask, amask);

    gPlayer[0].mode = MD_MENU; // 最初はメニュー画面なのでモードを設定
}

// 開放処理を行う関数
void opening_process()
{
    // 各スレッドが終了するまでmain関数の処理を中断
    SDL_WaitThread(wii_thread, NULL);      // wii_threadの処理終了を待つ
    SDL_WaitThread(keyboard_thread, NULL); // keyboard_threadの処理終了を待つ

    SDL_DestroyMutex(mtx); // Mutexを破棄

    // 終了処理
    SDL_DestroyRenderer(gGame.renderer); // RCの破棄（解放）
    SDL_DestroyWindow(gGame.window);     // 生成したウィンドウの破棄（消去）
    SDL_Quit();
}