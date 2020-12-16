#include "header/define.h"

/*******************************************************************
 * 関数名 : init_sys
 * 　　型 : void
 * 　引数 : int argc     引数の数
 *          char* argv[] WiiリモコンのMACアドレス
 * 　説明 : システムを初期化する関数
 ******************************************************************/
void init_sys(int argc, char* argv[])
{
    init_sdl2(); // SDL2を初期化する

    init_wiimote(argc, argv); // Wiiリモコンを初期化する

    gGame.mode = MODE_MENU; // モードをメニューに設定する
    selecter   = 0;         // セレクターを 0 に初期化する

    sprintf(gPlayer.name, "%s", "guest"); // プレイヤー名を仮で guest にする
    gPlayer.score = 0;                    // プレイヤーのスコアを０に初期化

    /* 的の初期化 */
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

    wiimote_ir_thread = SDL_CreateThread(wiimote_ir_func, "wii_ir_thread", NULL);
}

/*******************************************************************
 * 関数名 : init_sdl2
 * 　　型 : void
 * 　説明 : SDL2 を初期化する関数
 ******************************************************************/
void init_sdl2()
{
    /* SDL2 初期化 */
    SDL_Init(SDL_INIT_VIDEO);              // SDL初期化
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG); // SDL_IMG初期化
    TTF_Init();                            // SDL_TTF初期化

    /* フォント読み込み 10 ~ 50px */
    fonts.size10 = TTF_OpenFont(FONT_PATH, 10);
    fonts.size15 = TTF_OpenFont(FONT_PATH, 15);
    fonts.size20 = TTF_OpenFont(FONT_PATH, 20);
    fonts.size25 = TTF_OpenFont(FONT_PATH, 25);
    fonts.size50 = TTF_OpenFont(FONT_PATH, 50);

    /* 背景画像読み込み */
    image_bg[0] = IMG_Load("./image/bg1.jpg"); // プレイ中の背景
    image_bg[1] = IMG_Load("./image/bg2.jpg");
    image_bg[2] = IMG_Load("./image/bg3.png");
    image_bg[3] = IMG_Load("./image/bg4.png");

    image_menu = IMG_Load("./image/menu_bg.png"); // メニュー画面の背景

    /* 的の画像読み込み */
    image_target[0] = IMG_Load("./image/target/100-1.png");
    image_target[1] = IMG_Load("./image/target/200-1.png");
    image_target[2] = IMG_Load("./image/target/500-1.png");
    image_target[3] = IMG_Load("./image/target/1000-1.png");
    image_target[4] = IMG_Load("./image/target/2000-1.png");

    /* SDL初期化 */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        Error("SDLの初期化に失敗しました");
        exit(-1);
    }

    /* ウィンドウ生成 */
    if ((window = SDL_CreateWindow("21-SHOOT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0)) == NULL) {
        Error("ウィンドウの生成に失敗しました");
        exit(-1);
    }

    /* レンダリングコンテキスト（RC）作成 */
    if ((renderer = SDL_CreateRenderer(window, -1, 0)) == NULL) {
        Error("レンダラーの生成に失敗しました");
        exit(EXIT_FAILURE); // システム強制終了
    }

    /* Surface 用のマスクを設定 */
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

    /* 合成画像作成用サーフェイスを作成 */
    surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, rmask, gmask, bmask, amask);

    /* キーボード入力用のスレッドを起動 */
    gGame.status    = ACTIVE; // ゲームをアクティブにする
    keyboard_thread = SDL_CreateThread(keyboard_func, "keyboard_thread", NULL);
}

/*******************************************************************
 * 関数名 : init_wiimote
 * 　　型 : void
 * 　引数 : int argc     引数の数
 *          char* argv[] WiiリモコンのMACアドレス
 * 　説明 : Wiiリモコンを初期化する関数
 ******************************************************************/
void init_wiimote(int argc, char* argv[])
{
    /* Wiiリモコン処理 */
    if (argc < 2) { // Wiiリモコン識別情報がコマンド引数で与えられなければ
        Error("WiiリモコンのMACアドレスを引数に指定してください");
        exit(EXIT_FAILURE); // システム強制終了
    }

    // Wiiリモコンの接続待機中の画面
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 生成したRCに描画色として青を設定
    SDL_RenderClear(renderer);                      // 設定色でRCをクリア
    surface = TTF_RenderUTF8_Blended(fonts.size25, "Press buttons 1 and 2", (SDL_Color) { 255, 255, 255, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 100, 100, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);

    surface = TTF_RenderUTF8_Blended(fonts.size25, "on the wiimote now to connect.", (SDL_Color) { 255, 255, 255, 255 });
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 100, 150, iw, ih };
    SDL_RenderCopy(renderer, texture, &txtRect, &pasteRect);
    SDL_RenderPresent(renderer);

    /* Wiiリモコンの接続（１つのみ）コマンド引数に指定したWiiリモコン識別情報を渡して接続 */
    SystemLog("Wiiリモコンの接続試行中...");
    wiimote_connect(&wiimote, argv[1]);

    if (!wiimote_is_open(&wiimote)) {
        Error("Wiiリモコンの接続に失敗しました");
        Error("システムを終了します");
        exit(EXIT_FAILURE); // システム強制終了
    } else {
        SystemLog("Wiiリモコンの接続に成功しました");
    }

    wiimote.led.one  = 1; // WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
    wiimote.mode.acc = 1; // センサからのデータを受け付けるモードに変更
    wiimote.mode.ir  = 1; // 赤外線センサをON

    wiimote_update(&wiimote); // Wiiリモコンの状態更新

    /* Wiiリモコン入力用のスレッドを起動 */
    wiimote_thread = SDL_CreateThread(wiimote_func, "wiimote_thread", NULL);
}

/*******************************************************************
 * 関数名 : opening_sys
 * 　　型 : void
 * 　説明 : システムを開放する関数
 ******************************************************************/
void opening_sys()
{
    SystemLog("Wiiリモコンの接続を解除します");
    wiimote_disconnect(&wiimote); // Wiiリモコン接続解除

    /* 各スレッドが終了するまでmain関数の処理を中断 */
    SystemLog("各スレッドの終了待ち");
    SDL_WaitThread(keyboard_thread, NULL);   // keyboard_threadの処理終了を待つ
    SDL_WaitThread(wiimote_thread, NULL);    // wiimote_threadの処理終了を待つ
    SDL_WaitThread(wiimote_ir_thread, NULL); // wiimote_ir_threadの処理終了を待つ

    SDL_RemoveTimer(timer_id_countdown);
    SDL_RemoveTimer(timer_id_transition_stage);

    SystemLog("レンダラーとウィンドウを破棄します");
    SDL_DestroyRenderer(renderer); // RC の破棄（解放）
    SDL_DestroyWindow(window);     // 生成したウィンドウの破棄（消去）
    SDL_Quit();                    // SDL を終了する

    /* フォントをクローズ */
    TTF_CloseFont(fonts.size10);
    TTF_CloseFont(fonts.size15);
    TTF_CloseFont(fonts.size20);
    TTF_CloseFont(fonts.size25);
    TTF_CloseFont(fonts.size50);
    TTF_Quit(); // TTF を終了する
}