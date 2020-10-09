#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

m #include<SDL2 / SDL.h>             // SDLを用いるために必要なヘッダファイル
#include <SDL2/SDL2_gfxPrimitives.h> // 描画関係のヘッダファイル

#include <libcwiimote/wiimote.h>     // Wiiリモコンを用いるために必要なヘッダファイル
#include <libcwiimote/wiimote_api.h> // Wiiリモコンを用いるために必要なヘッダファイル

    // ウインドウサイズ
    enum {
        WD_Width  = 1000,
        WD_Height = 400
    };

extern SDL_Window* window;          // ウィンドウデータを格納する構造体
extern SDL_Renderer* renderer;      // 2Dレンダリングコンテキスト（描画設定）を格納する構造体
extern SDL_Thread* wii_thread;      // wii_threadを用いる
extern SDL_Thread* keyboard_thread; // keyboard_threadを用いる
extern SDL_mutex* mtx;              // 相互排除（Mutex）
extern SDL_Event event;             // SDLによるイベントを検知するための構造体

extern wiimote_t wiimote; // Wiiリモコンの状態格納用

extern void init_sys();               // SDLやWiiリモコンを初期化する関数
extern int wii_func(void* args);      // Wiiリモコンの入力制御関数
extern int keyboard_func(void* args); // キーボードの入力制御関数

extern void Error(); // エラーを色付きで出力する関数
extern void Log();   // ログを色付きで出力する関数

// カラーコード
#define COLOR_BG_BLACK "\x1b[40m"
#define COLOR_BG_RED "\x1b[41m"
#define COLOR_BG_GREEN "\x1b[42m"
#define COLOR_BG_YELLOW "\x1b[43m"
#define COLOR_BG_BLUE "\x1b[44m"
#define COLOR_BG_MAGENTA "\x1b[45m"
#define COLOR_BG_CYAN "\x1b[46m"
#define COLOR_BG_WHITE "\x1b[47m"

#define COLOR_FG_BLACK "\x1b[30m"
#define COLOR_FG_RED "\x1b[31m"
#define COLOR_FG_GREEN "\x1b[32m"
#define COLOR_FG_YELLOW "\x1b[33m"
#define COLOR_FG_BLUE "\x1b[34m"
#define COLOR_FG_MAGENTA "\x1b[35m"
#define COLOR_FG_CYAN "\x1b[36m"
#define COLOR_FG_WHITE "\x1b[37m"

#define COLOR_BG_LBLACK "\x1b[100m"
#define COLOR_BG_LRED "\x1b[101m"
#define COLOR_BG_LGREEN "\x1b[102m"
#define COLOR_BG_LYELLOW "\x1b[103m"
#define COLOR_BG_LBLUE "\x1b[104m"
#define COLOR_BG_LMAGENTA "\x1b[105m"
#define COLOR_BG_LCYAN "\x1b[106m"
#define COLOR_BG_LWHITE "\x1b[107m"

#define COLOR_FG_LBLACK "\x1b[90m"
#define COLOR_FG_LRED "\x1b[91m"
#define COLOR_FG_LGREEN "\x1b[92m"
#define COLOR_FG_LYELLOW "\x1b[93m"
#define COLOR_FG_LBLUE "\x1b[94m"
#define COLOR_FG_LMAGENTA "\x1b[95m"
#define COLOR_FG_LCYAN "\x1b[96m"
#define COLOR_FG_LWHITE "\x1b[97m"

#define COLOR_REVERSE "\x1b[7m"
#define COLOR_UNDERLINE "\x1b[4m"
#define COLOR_BOLD "\x1b[1m"
#define COLOR_RESET "\x1b[0m"

#endif