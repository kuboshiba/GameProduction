#include "header/define.h"

SDL_Window* window;          // ウィンドウデータを格納する構造体
SDL_Renderer* renderer;      // 2Dレンダリングコンテキスト（描画設定）を格納する構造体
SDL_Thread* wii_thread;      // wii_threadを用いる
SDL_Thread* keyboard_thread; // keyboard_threadを用いる
SDL_mutex* mtx;              // 相互排除（Mutex）

SDL_Event event; // SDLによるイベントを検知するための構造体

wiimote_t wiimote = WIIMOTE_INIT; // Wiiリモコンの状態格納用

// SDLやWiiリモコンを初期化する関数
void init_sys(int argc, char* argv[])
{
    // SDL初期化
    SDL_Init(SDL_INIT_VIDEO);

    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        Error("SDLの初期化に失敗しました");
        exit(-1);
    }

    // Wiiリモコン初期化
    if (argc < 2) {
        Error("WiiリモコンのMACアドレスを引数に指定してください");
        exit(1);
    }

    // コマンド引数に指定したWiiリモコン識別情報を渡して接続
    if (wiimote_connect(&wiimote, argv[1]) < 0) {
        printf("unable to open wiimote: %s\n", wiimote_get_error());
        exit(1);
    }

    wiimote.led.one  = 1; // WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
    wiimote.mode.acc = 1; // センサからのデータを受け付けるモードに変更

    wiimote_update(&wiimote); // Wiiリモコンの状態更新

    // ウィンドウ生成
    if ((window = SDL_CreateWindow("No Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WD_Width, WD_Height, 0)) == NULL) {
        printf("failed to create window.\n");
        exit(-1);
    }

    // レンダリングコンテキスト（RC）作成
    if ((renderer = SDL_CreateRenderer(window, -1, 0)) == NULL) {
        printf("failed to create renderer.\n");
        exit(-1);
    }

    // 相互排除（Mutex）あり
    mtx = SDL_CreateMutex(); // 相互排除（Mutex）を用いる

    // スレッドを作成・実行
    wii_thread      = SDL_CreateThread(wii_func, "wii_thread", mtx);
    keyboard_thread = SDL_CreateThread(keyboard_func, "keyboard_thread", mtx);

    while (wiimote_is_open(&wiimote)) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderPresent(renderer);
    }

    // 各スレッドが終了するまでmain関数の処理を中断
    SDL_WaitThread(wii_thread, NULL);      // wii_threadの処理終了を待つ
    SDL_WaitThread(keyboard_thread, NULL); // keyboard_threadの処理終了を待つ

    SDL_DestroyMutex(mtx); // Mutexを破棄

    // 終了処理
    SDL_DestroyRenderer(renderer); // RCの破棄（解放）
    SDL_DestroyWindow(window);     // 生成したウィンドウの破棄（消去）
    SDL_Quit();
}