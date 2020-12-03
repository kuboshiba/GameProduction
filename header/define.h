#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <arpa/inet.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>                // SDLを用いるために必要なヘッダファイル
#include <SDL2/SDL2_gfxPrimitives.h> // 描画関係のヘッダファイル
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <libcwiimote/wiimote.h>     // Wiiリモコンを用いるために必要なヘッダファイル
#include <libcwiimote/wiimote_api.h> // Wiiリモコンを用いるために必要なヘッダファイル

// define
#define FONT_PATH "./font/PressStart2P-Regular.ttf" // フォントのパス
#define TARGET_NUM_MAX 10

#define DEFAULT_PORT 51000 // デフォルトのポート番号
#define MAX_LEN_NAME 10    // 名前の最大文字数
#define MAX_NUM_CLIENTS 5  // クライアントの最大数
#define MAX_LEN_BUFFER 256 // バッファーの最大値
#define MAX_LEN_ADDR 32    // アドレスの最大値
#define BROADCAST -1       // デフォルトのブロードキャスト値

#define MESSAGE_COMMAND 'M' // メッセージコマンド
#define QUIT_COMMAND 'Q'    // 終了コマンド
#define DATA_COMMAND 'D'    // データ送信コマンド

// ウインドウサイズ
enum {
    WD_Width  = 1000,
    WD_Height = 500
};

enum {
    SEL_OK     = 0,
    SEL_CANCEL = 1,
    SEL_HOST   = 0,
    SEL_CLIENT = 1,
};

// 画面のモード
typedef enum {
    MD_MENU              = 0, // メニュー
    MD_SOLO_WAIT         = 1, // ソロプレイ待機
    MD_SOLO_PLAYING_1    = 2, // ソロプレイ中
    MD_SOLO_PLAYING_2    = 3, // ソロプレイのゲーム中
    MD_MULTI_WAIT        = 4, // マルチプレイ待機
    MD_MULTI_HOST_1      = 5,
    MD_MULTI_HOST_2      = 6,
    MD_MULTI_HOST_3      = 7,
    MD_MULTI_CLIENT      = 8,
    MD_MULTI_PLAYING     = 9,  // マルチプレイ中
    MD_PLAYER_NAME_INPUT = 96, // プレイヤーネームの入力
    MD_EXIT_WAIT         = 97, // 終了待機
    MD_EXIT              = 98, // 終了
    MD_WAIT              = 99  // 待機
} MODE;

// ゲームの状態
typedef struct {
    SDL_Window* window;     // ウィンドウデータを格納する構造体
    SDL_Renderer* renderer; // 2Dレンダリングコンテキスト（描画設定）を格納する構造体
    SDL_Surface* surface;   // サーフェイス（メインメモリ上の描画データ）を格納する構造体
    SDL_Texture* texture;   // テクスチャ（VRAM上の描画データ）を格納する構造体
    MODE mode;
    char name[100];
    int score;
} GameInfo;

// プレイヤーの状態
typedef struct {
    MODE mode;
    int score;
    char name[100];
} Player;

typedef struct {
    int type;
    int x;
    int y;
    int cnt;
} Target;

// クライアントの構造体
typedef struct {
    int cid;
    int sock;
    struct sockaddr_in addr;
    char name[MAX_LEN_NAME];
} CLIENT;

// コンテナの構造体
typedef struct {
    int cid;
    char command;
    char message[MAX_LEN_BUFFER];
} CONTAINER;

extern GameInfo gGame;    // ゲームの状態
extern Player gPlayer[4]; // プレイヤーの状態
extern Target target[10];

extern SDL_Thread* wii_thread;            // wii_threadを用いる
extern SDL_Thread* keyboard_thread;       // keyboard_threadを用いる
extern SDL_Thread* wii_ir_thread;         // wii_ir_threadを用いる
extern SDL_Thread* network_host_thread;   // network_host_threadを用いる
extern SDL_Thread* network_client_thread; // network_client_threadを用いる

extern SDL_mutex* mtx;  // 相互排除（Mutex）
extern SDL_Event event; // SDLによるイベントを検知するための構造体

extern SDL_TimerID timer_id_1; // min_flips_callback用のタイマー
extern SDL_TimerID timer_id_2; // カウントダウン用

extern SDL_Surface* image_bg_1;       // 背景画像用のサーフェイス
extern SDL_Surface* image_bg_2;       // 背景画像用のサーフェイス
extern SDL_Surface* image_bg_3;       // 背景画像用のサーフェイス
extern SDL_Surface* image_bg_4;       // メニュー画像用のサーフェイス
extern SDL_Surface* image_menu_bg;    // メニュー画像陽のサーフェイス
extern SDL_Surface* image_target[10]; // 的の画像用のサーフェイス

extern SDL_Rect src_rect_bg; // 画像の切り取り範囲
extern SDL_Rect dst_rect_bg; // 画像の描画位置
extern SDL_Rect txtRect;     // 文字を描画する際に使用
extern SDL_Rect pasteRect;   // 文字を描画する際に使用
extern SDL_Rect pointer;
extern SDL_Rect pointer_prev;

extern TTF_Font* font10;
extern TTF_Font* font18;
extern TTF_Font* font20;
extern TTF_Font* font25; // TrueTypeフォントデータを格納する構造体
extern TTF_Font* font50; // TrueTypeフォントデータを格納する構造体

extern wiimote_t wiimote; // Wiiリモコンの状態格納用

extern Uint32 rmask, gmask, bmask, amask; // サーフェイス作成時のマスクデータを格納する変数

extern int iw, ih;        // 文字を描画する際に使用
extern int player_num;    // プレイヤーの数
extern int menu_sel;      // メニューのボタンのセレクト位置
extern int menu_sel_num;  // メニューのボタンのセレクトの数
extern int alpha_key_pos; // キーボード入力のセレクタ

extern char alpha[27][2];

extern bool flag_loop;    // メインループのループフラグ
extern bool flag_subloop; // メインループのループフラグ

// server.c
extern CLIENT s_clients[MAX_NUM_CLIENTS]; // 構造体 CLIENT を構造体配列 s_clients
extern CONTAINER s_data;                  // 構造体 DATA を構造体変数 s_data で宣言
extern int s_num_clients;                 // クライアントの数を格納
extern int s_num_socks;                   // ソケットの数を格納
extern fd_set s_mask;

// client.c
extern int c_sock;
extern int c_num_clients;
extern int c_myid;
extern int c_num_sock;
extern fd_set c_mask;
extern CLIENT c_clients[MAX_NUM_CLIENTS];

// system.c
extern void init_sys();        // SDLやWiiリモコンを初期化する関数
extern void opening_process(); // 開放処理を行う関数

// input.c
extern int wii_func();      // Wiiリモコンの入力制御関数
extern int keyboard_func(); // キーボードの入力制御関数
extern int wii_ir_func();   // Wiiリモコンの赤外線センサの入力制御関数

extern void md_multi_host(int player_num);

// define.c
extern void Error();                                                     // エラーを色付きで出力する関数
extern void Log();                                                       // ログを色付きで出力する関数
extern void SystemLog();                                                 // ログを色付きで出力する関数
extern int map(int x, int in_min, int in_max, int out_min, int out_max); // map関数

// server.c
extern int server_main();
extern void setup_server(int, u_short);
extern int server_control_requests();
extern void server_send_data(int cid, void* data, int size);
extern int server_receive_data(int cid, void* data, int size);
extern void terminate_server(void);
extern void server_handle_error(char* message);

// client.c
extern int client_main();
extern void setup_client(char* server_name, u_short port);
extern int client_control_requests();
extern int in_command(void);
extern int exe_command(void);
extern void client_send_data(void*, int);
extern int client_receive_data(void*, int);
extern void client_handle_error(char*);
extern void terminate_client();

// client.c
extern int client_main();

// color code
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