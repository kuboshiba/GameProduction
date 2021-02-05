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

#include <SDL2/SDL.h>                // SDL2 標準 API
#include <SDL2/SDL2_gfxPrimitives.h> // SDL2 図形描画系
#include <SDL2/SDL_image.h>          // SDL2 画像描画系
#include <SDL2/SDL_mixer.h>          // SDL2 音声再生系
#include <SDL2/SDL_ttf.h>            // SDL2 文字描画系

#include <libcwiimote/wiimote.h>     // Wiiリモコンを用いるために必要なヘッダファイル
#include <libcwiimote/wiimote_api.h> // Wiiリモコンを用いるために必要なヘッダファイル

#define SCREEN_WIDTH 1000 // ウィンドウの横サイズ
#define SCREEN_HEIGHT 500 // ウィンドウの縦サイズ

#define FONT_PATH "./font/PressStart2P-Regular.ttf" // フォントのパス

#define MODE_NUM 100  // モードの個数
#define STAGE_TIME 15 // ステージごとの時間 sec

#define DEFAULT_PORT 51000 // デフォルトのポート番号
#define MAX_LEN_NAME 100   // 名前の最大文字数
#define MAX_NUM_CLIENTS 4  // クライアントの最大数
#define MAX_LEN_BUFFER 256 // バッファーの最大値
#define MAX_LEN_ADDR 32    // アドレスの最大値
#define BROADCAST -1       // デフォルトのブロードキャスト値

#define MESSAGE_COMMAND 'A' // メッセージコマンド
#define QUIT_COMMAND 'B'    // 終了コマンド
#define DATA_COMMAND 'C'    // データ送信コマンド
#define DATA_TARGET_COMMAND 'D'
#define START_COMMAND 'E' // スタートコマンド
#define SYNC_COMMAND 'F'
#define C_TO_S_TARGET_COMMAND 'G'

#define TARGET_NUM_MAX 15
#define SERVER_ADDR "192.168.64.34" // サーバーのアドレス

typedef enum {
    PASSIVE = 0, // ゲームがパッシブ
    ACTIVE  = 1  // ゲームがアクティブ
} STATUS;

typedef enum {
    MODE_MENU                         = 0,  // メニュー画面
    MODE_SOLO_OK_OR_CANCEL            = 1,  // ソロプレイ select OK or CANCEL
    MODE_INPUT_NAME                   = 2,  // プレイヤー名入力
    MODE_SOLO_PLAYING                 = 3,  // ソロプレイ　プレイ中
    MODE_MULTI_HOST_OR_CLIENT         = 4,  // マルチプレイ　ホストかクライアントか選択
    MODE_MULTI_HOST_PLAYER_NUM_DECIDE = 5,  // マルチプレイ　ホスト選択時人数決定
    MODE_MULTI_HOST_SERVER_SETUP      = 6,  // サーバーセットアップ
    MODE_MULTI_CLIENT_INPUT_NAME      = 7,  // クライアントの名前入力
    MODE_MULTI_CLIENT_SETUP           = 8,  // クライアントのセットアップ
    MODE_MULTI_CLIENT_WAIT            = 9,  // クライアントの待機
    MODE_MULTI_PLAYING                = 10, // マルチプレイ　プレイ中
    MODE_MULTI_PLAYING_WAIT           = 11, // マルチプレイ　同期待ち
    MODE_COUNTDOWN                    = 20, // ソロプレイ　カウントダウン中
    MODE_TRANSITION                   = 21, // ステージ遷移
    MODE_RESULT                       = 22, // リザルト
    MODE_SETTING                      = 23, // 設定画面
} MODE;

/* ゲームで多用する共通変数は構造体 Game で管理 */
typedef struct {
    MODE mode;
    int player_num;
    int type;
} GameInfo;
extern GameInfo gGame; // ゲームで多用する共通変数は構造体 gGame で管理

typedef struct {
    char name[MAX_LEN_NAME];
    int score;
} PLAYER;
extern PLAYER gPlayer; // プレイヤーの情報

typedef struct {
    int x;
    int y;
} POINT; // 座標

typedef struct {
    int type;
    int x;
    int y;
    int cnt;
    int type_buf;
    int c_myid;
} Target;
extern Target target[TARGET_NUM_MAX]; // 的の情報

typedef struct {
    int cid;
    int sock;
    struct sockaddr_in addr;
    char name[MAX_LEN_NAME];
} CLIENT;                                 // クライアントの構造体
extern CLIENT s_clients[MAX_NUM_CLIENTS]; // 構造体 CLIENT を構造体配列 s_clients
extern CLIENT c_clients[MAX_NUM_CLIENTS];

typedef struct {
    int cid;
    char command;
    char message[MAX_LEN_BUFFER];
    int score[MAX_NUM_CLIENTS];
    Target target[TARGET_NUM_MAX];
} CONTAINER; // コンテナの構造体
extern CONTAINER data;
extern CONTAINER s_data; // 構造体 DATA を構造体変数 s_data で宣言
extern CONTAINER c_data; // 構造体 DATA を構造体変数 s_data で宣言

typedef struct {
    SDL_Surface *frame[10];
} IMAGE_TARGET_ANIME;
extern IMAGE_TARGET_ANIME image_target_anime[TARGET_NUM_MAX];

/* SDL 関係 */
extern SDL_Window *window;     // ウィンドウデータを格納する構造体
extern SDL_Renderer *renderer; // 2Dレンダリングコンテキスト（描画設定）を格納する構造体
extern SDL_Surface *surface;   // サーフェイス（メインメモリ上の描画データ）を格納する構造体
extern SDL_Texture *texture;   // テクスチャ（VRAM上の描画データ）を格納する構造体
extern SDL_Event event;        // SDLによるイベントを検知するための構造体

/*  PressStart2Pのフォント格納データ */
typedef struct {
    TTF_Font *size10;
    TTF_Font *size15;
    TTF_Font *size20;
    TTF_Font *size25;
    TTF_Font *size50;
} Font_PressStart2P;
extern Font_PressStart2P fonts; // PressStart2Pのフォント格納データ

/* スレッド関係の定義・変数 */
extern SDL_Thread *keyboard_thread;       // キーボード入力用のスレッド
extern SDL_Thread *wiimote_thread;        // Wiiリモコン入力用のスレッド
extern SDL_Thread *wiimote_ir_thread;     // Wiiリモコンの赤外線センサの取得とポインター生成のスレッド
extern SDL_Thread *network_host_thread;   // network_host_threadを用いる
extern SDL_Thread *network_client_thread; // network_client_threadを用いる

/* タイマー関係の定義・変数 */
extern SDL_TimerID timer_id_countdown;                        // カウントダウン用のタイマー
extern SDL_TimerID timer_id_transition_stage;                 // ステージ遷移用のタイマー
extern SDL_TimerID timer_id_target;                           // 的の生成タイマー
extern SDL_TimerID timer_id_target_animation[TARGET_NUM_MAX]; // アニメーションの生成タイマー
extern Uint32 animation_target_func(Uint32, void *);

/* MUSIC */
extern Mix_Music *bgm_menu; // BGM ファイルを読み込む構造体
extern int music_volume;    // BGM の音量

/* 画像関係の定義・変数 */
#define IMAGE_BG_NUM 4                              // 背景画像の数
#define IMAGE_TARGET_NUM 5                          // 的の画像の数
extern SDL_Surface *image_bg[IMAGE_BG_NUM];         // 背景画像
extern SDL_Surface *image_target[IMAGE_TARGET_NUM]; // 的の画像
extern SDL_Surface *image_menu;                     // メニュー画像
extern SDL_Surface *image_rect[10];                 // ゲーム中のステータスを表示する背景
extern SDL_Rect imageRect;                          // 画像の選択範囲
extern SDL_Rect drawRect;                           // 画像の描画位置
extern SDL_Rect txtRect;                            // 文字の選択範囲
extern SDL_Rect pasteRect;                          // 文字の描画範囲
extern Uint32 rmask, gmask, bmask, amask;           // サーフェイス作成時のマスクデータを格納する変数
extern int iw, ih;                                  // テクスチャやサーフェイスの幅

typedef enum {
    OBJECT_TYPE_STATIC = 0,
    OBJECT_TYPE_CLOUD  = 1
} OBJECT_TYPE;

#define OBJECT_NUM_MAX 50
typedef struct {
    SDL_Surface *background;             // バックグラウンド
    SDL_Surface *object[OBJECT_NUM_MAX]; // オブジェクト
    int object_x[OBJECT_NUM_MAX];
    int object_y[OBJECT_NUM_MAX];
    OBJECT_TYPE object_type[OBJECT_NUM_MAX];
    int object_num; // オブジェクトの数
} IMAGE_OBJECT;
extern IMAGE_OBJECT image[IMAGE_BG_NUM]; // 画像をステージごとにまとめた構造体

/* Wiiリモコン関係 */
extern wiimote_t wiimote;     // Wiiリモコンの状態格納用
extern SDL_Rect pointer;      // Wiiリモコンの赤外線センサーの座標
extern SDL_Rect pointer_prev; // Wiiリモコンの赤外線センサーの前の座標

/* その他 */
extern int selecter;         // セレクター
extern int key_pos;          // プレイヤー名入力画面のセレクター
extern char alphabet[27][2]; // アルファベット
extern bool flag[100];       // フラグ

/**********************************************************************
******************************* server.c ******************************
**********************************************************************/
extern int s_num_clients; // クライアントの数を格納
extern int s_num_socks;   // ソケットの数を格納
extern fd_set s_mask;     // サーバーのマスク
extern int flag_sync;     // 同期用のフラグ

extern int server_main();
extern void setup_server(int, u_short);
extern int server_control_requests();
extern void server_send_data(int cid, void *data, int size);
extern int server_receive_data(int cid, void *data, int size);
extern void terminate_server(void);
extern void server_handle_error(char *message);

/**********************************************************************
******************************* client.c ******************************
**********************************************************************/
extern int c_sock;        // クライアントのソケット
extern int c_num_clients; // クライアントの数を格納
extern int c_myid;        // クライアントのID
extern int c_num_sock;    // クライアントのソケットの数
extern fd_set c_mask;     // クライアントのマスク

extern int client_main();
extern void setup_client(char *server_name, u_short port);
extern int client_control_requests();
extern int in_command(void);
extern int exe_command(void);
extern void client_send_data(void *, int);
extern int client_receive_data(void *, int);
extern void client_handle_error(char *);
extern void terminate_client();

/**********************************************************************
******************************* system.c ******************************
**********************************************************************/
/* system.c 関数 */
extern void init_sys(int argc, char *argv[]);     // システムを初期化する関数
extern void init_sdl2();                          // SDL2 を初期化する関数
extern void init_wiimote(int argc, char *argv[]); // Wiiリモコンを初期化する関数
extern void opening_sys();                        // システムを開放する関数

/**********************************************************************
******************************+* input.c ******************************
**********************************************************************/
/* input.c 関数 */
extern int keyboard_func();   // キーボード入力用の関数
extern int wiimote_func();    // Wiiリモコン入力用の関数
extern int wiimote_ir_func(); // Wiiリモコン入力用の関数

/**********************************************************************
******************************* define.c ******************************
**********************************************************************/
/*  define.c 関数 */
extern void Error(char *);     // エラーを色付きで出力する関数
extern void Log(char *);       // ログを色付きで出力する関数
extern void SystemLog(char *); // ログを色付きで出力する関数

extern int map(int x, int in_min, int in_max, int out_min, int out_max); // map関数

/* define.c 定義 */
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