#include "header/define.h"

// SDLやWiiリモコンを初期化する関数
void init_sys(int argc, char* argv[])
{
    // SDL初期化
    SDL_Init(SDL_INIT_VIDEO);
    // SDL_IMG初期化
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    // SDL_TTF初期化
    TTF_Init();

    font10 = TTF_OpenFont(FONT_PATH, 10); // フォントサイズ25読み込み
    font18 = TTF_OpenFont(FONT_PATH, 18); // フォントサイズ25読み込み
    font20 = TTF_OpenFont(FONT_PATH, 20); // フォントサイズ25読み込み
    font25 = TTF_OpenFont(FONT_PATH, 25); // フォントサイズ25読み込み
    font50 = TTF_OpenFont(FONT_PATH, 50); // フォントサイズ50読み込み

    image_bg_1    = IMG_Load("./image/bg1.jpg");     // 背景画像読み込み
    image_bg_2    = IMG_Load("./image/bg2.jpg");     // 背景画像読み込み
    image_bg_3    = IMG_Load("./image/bg3.png");     // 背景画像読み込み
    image_bg_4    = IMG_Load("./image/bg4.png");     // 背景画像読み込み
    image_menu_bg = IMG_Load("./image/menu_bg.png"); // メニュー背景画像読み込み

    image_target[0] = IMG_Load("./image/target/100-1.png");
    image_target[1] = IMG_Load("./image/target/200-1.png");
    image_target[2] = IMG_Load("./image/target/500-1.png");
    image_target[3] = IMG_Load("./image/target/1000-1.png");
    image_target[4] = IMG_Load("./image/target/2000-1.png");

    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        Error("SDLの初期化に失敗しました");
        exit(-1);
    }

    // ウィンドウ生成
    if ((gGame.window = SDL_CreateWindow("21-SHOOT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WD_Width, WD_Height, 0)) == NULL) {
        Error("ウィンドウの生成に失敗しました");
        exit(-1);
    }

    // レンダリングコンテキスト（RC）作成
    if ((gGame.renderer = SDL_CreateRenderer(gGame.window, -1, 0)) == NULL) {
        Error("レンダラーの生成に失敗しました");
        exit(-1);
    }

    // 初期画面
    SDL_SetRenderDrawColor(gGame.renderer, 0, 0, 0, 255); // 生成したRCに描画色として青を設定
    SDL_RenderClear(gGame.renderer);                      // 設定色でRCをクリア
    gGame.surface = TTF_RenderUTF8_Blended(font25, "Press buttons 1 and 2", (SDL_Color) { 255, 255, 255, 255 });
    gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
    SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 100, 100, iw, ih };
    SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);

    gGame.surface = TTF_RenderUTF8_Blended(font25, "on the wiimote now to connect.", (SDL_Color) { 255, 255, 255, 255 });
    gGame.texture = SDL_CreateTextureFromSurface(gGame.renderer, gGame.surface);
    SDL_QueryTexture(gGame.texture, NULL, NULL, &iw, &ih);
    txtRect   = (SDL_Rect) { 0, 0, iw, ih };
    pasteRect = (SDL_Rect) { 100, 150, iw, ih };
    SDL_RenderCopy(gGame.renderer, gGame.texture, &txtRect, &pasteRect);
    SDL_RenderPresent(gGame.renderer);

    // Wiiリモコン処理
    if (argc < 2) { // Wiiリモコン識別情報がコマンド引数で与えられなければ
        Error("WiiリモコンのMACアドレスを引数に指定してください");
        exit(1);
    }

    // Wiiリモコンの接続（１つのみ）
    // コマンド引数に指定したWiiリモコン識別情報を渡して接続
    SystemLog("Wiiリモコンの接続試行中...");
    wiimote_connect(&wiimote, argv[1]);

    if (!wiimote_is_open(&wiimote)) {
        Error("Wiiリモコンの接続に失敗しました");
        exit(1);
    }

    SystemLog("Wiiリモコンの接続に成功しました");

    wiimote.led.one  = 1; // WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
    wiimote.mode.acc = 1; // センサからのデータを受け付けるモードに変更
    wiimote.mode.ir  = 1; // 赤外線センサをON

    wiimote_update(&wiimote); // Wiiリモコンの状態更新

    // 相互排除（Mutex）あり
    mtx = SDL_CreateMutex(); // 相互排除（Mutex）を用いる

    // スレッドを作成・実行
    wii_thread      = SDL_CreateThread(wii_func, "wii_thread", NULL);
    keyboard_thread = SDL_CreateThread(keyboard_func, "keyboard_thread", NULL);
    wii_ir_thread   = SDL_CreateThread(wii_ir_func, "wii_ir_thread", NULL);

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

    // 状態初期化
    gGame.mode = MD_MENU;
    for (int i = 0; i < 4; i++) {
        gGame.mode  = MD_MENU; // 最初はメニュー画面なのでモードを設定
        gGame.score = 0;       // スコアは0に設定
        sprintf(gGame.name, "%s", "guest");
    }

    // 的の初期化
    for (int i = 0; i < 10; i++) {
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
}

// 開放処理を行う関数
void opening_process()
{
    SystemLog("Wiiリモコンの接続を解除します");
    wiimote_disconnect(&wiimote); // Wiiリモコン接続解除

    // 各スレッドが終了するまでmain関数の処理を中断
    SystemLog("各スレッドの終了待ち");
    SDL_WaitThread(wii_thread, NULL);      // wii_threadの処理終了を待つ
    SDL_WaitThread(keyboard_thread, NULL); // keyboard_threadの処理終了を待つ
    SDL_WaitThread(wii_ir_thread, NULL);   // wii_ir_threadの処理終了を待つ

    SDL_RemoveTimer(timer_id_1);
    SDL_RemoveTimer(timer_id_2);

    SystemLog("Mutexを破棄します");
    SDL_DestroyMutex(mtx); // Mutexを破棄

    // 終了処理
    SystemLog("レンダラーとウィンドウを破棄します");
    SDL_DestroyRenderer(gGame.renderer); // RCの破棄（解放）
    SDL_DestroyWindow(gGame.window);     // 生成したウィンドウの破棄（消去）
    SDL_Quit();

    TTF_CloseFont(font25);
    TTF_CloseFont(font50);
    TTF_Quit();
}