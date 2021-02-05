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

    keyboard_thread   = SDL_CreateThread(keyboard_func, "keyboard_thread", NULL); // キーボード入力スレッド作成
    wiimote_ir_thread = SDL_CreateThread(wiimote_ir_func, "wii_ir_thread", NULL); // Wiiリモコンの入力スレッド作成

    /* 音楽関係初期化・設定 */
    Mix_OpenAudio(22050, AUDIO_S16, 2, 4096);
    bgm_menu     = Mix_LoadMUS("sound/music/title_01.mp3");
    music_volume = 12;
    Mix_AllocateChannels(16);
    Mix_PlayMusic(bgm_menu, -1);
    Mix_VolumeMusic(music_volume);

    gGame.mode = MODE_MENU; // モードをメニューに設定する
    gGame.type = 0;
    selecter   = 0; // セレクターを 0 に初期化する

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

    flag_sync = 0;
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

    /* ステージ１の画像読み込み */
    image[0].background = IMG_Load("./image/none/bg.png");

    image[0].object[0]      = IMG_Load("./image/none/cloud1.png");
    image[0].object_x[0]    = 63;
    image[0].object_y[0]    = 13;
    image[0].object_type[0] = OBJECT_TYPE_CLOUD;
    image[0].object[1]      = IMG_Load("./image/none/cloud1.png");
    image[0].object_x[1]    = 294;
    image[0].object_y[1]    = 32;
    image[0].object_type[1] = OBJECT_TYPE_CLOUD;
    image[0].object[2]      = IMG_Load("./image/none/cloud1.png");
    image[0].object_x[2]    = 645;
    image[0].object_y[2]    = 17;
    image[0].object_type[2] = OBJECT_TYPE_CLOUD;
    image[0].object[3]      = IMG_Load("./image/stage1/stage1_box.png");
    image[0].object_x[3]    = 525;
    image[0].object_y[3]    = 281;
    image[0].object_type[3] = OBJECT_TYPE_STATIC;
    image[0].object[4]      = IMG_Load("./image/stage1/stage1_tree.png");
    image[0].object_x[4]    = 75;
    image[0].object_y[4]    = 220;
    image[0].object_type[4] = OBJECT_TYPE_STATIC;

    image[0].object_num = 5;

    /* ステージ２の画像読み込み */
    image[1].background = IMG_Load("./image/none/bg.png");

    image[1].object[0]      = IMG_Load("./image/none/cloud1.png");
    image[1].object_x[0]    = 294;
    image[1].object_y[0]    = 32;
    image[1].object_type[0] = OBJECT_TYPE_CLOUD;
    image[1].object[1]      = IMG_Load("./image/none/cloud1.png");
    image[1].object_x[1]    = 645;
    image[1].object_y[1]    = 17;
    image[1].object_type[1] = OBJECT_TYPE_CLOUD;
    image[1].object[2]      = IMG_Load("./image/stage2/stage2_flower1.png");
    image[1].object_x[2]    = 500;
    image[1].object_y[2]    = 375;
    image[1].object_type[2] = OBJECT_TYPE_STATIC;
    image[1].object[3]      = IMG_Load("./image/stage2/stage2_flower2.png");
    image[1].object_x[3]    = 650;
    image[1].object_y[3]    = 375;
    image[1].object_type[3] = OBJECT_TYPE_STATIC;
    image[1].object[4]      = IMG_Load("./image/stage2/stage2_flower3.png");
    image[1].object_x[4]    = 800;
    image[1].object_y[4]    = 375;
    image[1].object_type[4] = OBJECT_TYPE_STATIC;
    image[1].object[5]      = IMG_Load("./image/stage2/stage2_house.png");
    image[1].object_x[5]    = 50;
    image[1].object_y[5]    = 1;
    image[1].object_type[5] = OBJECT_TYPE_STATIC;

    image[1].object_num = 6;

    /* ステージ３の画像読み込み */
    image[2].background = IMG_Load("./image/none/bg.png");

    image[2].object[0]       = IMG_Load("./image/none/cloud1.png");
    image[2].object_x[0]     = 63;
    image[2].object_y[0]     = 13;
    image[2].object_type[0]  = OBJECT_TYPE_CLOUD;
    image[2].object[1]       = IMG_Load("./image/none/cloud1.png");
    image[2].object_x[1]     = 294;
    image[2].object_y[1]     = 32;
    image[2].object_type[1]  = OBJECT_TYPE_CLOUD;
    image[2].object[2]       = IMG_Load("./image/none/cloud1.png");
    image[2].object_x[2]     = 645;
    image[2].object_y[2]     = 17;
    image[2].object_type[2]  = OBJECT_TYPE_STATIC;
    image[2].object[3]       = IMG_Load("./image/stage3/stage3_grave.png");
    image[2].object_x[3]     = 450;
    image[2].object_y[3]     = 413;
    image[2].object_type[3]  = OBJECT_TYPE_STATIC;
    image[2].object[4]       = IMG_Load("./image/stage3/stage3_grave.png");
    image[2].object_x[4]     = 550;
    image[2].object_y[4]     = 413;
    image[2].object_type[4]  = OBJECT_TYPE_STATIC;
    image[2].object[5]       = IMG_Load("./image/stage3/stage3_house1.png");
    image[2].object_x[5]     = 30;
    image[2].object_y[5]     = 131;
    image[2].object_type[5]  = OBJECT_TYPE_STATIC;
    image[2].object[6]       = IMG_Load("./image/stage3/stage3_house2.png");
    image[2].object_x[6]     = 280;
    image[2].object_y[6]     = 225;
    image[2].object_type[6]  = OBJECT_TYPE_STATIC;
    image[2].object[7]       = IMG_Load("./image/stage3/stage3_juuzi1.png");
    image[2].object_x[7]     = 650;
    image[2].object_y[7]     = 388;
    image[2].object_type[7]  = OBJECT_TYPE_STATIC;
    image[2].object[8]       = IMG_Load("./image/stage3/stage3_juuzi2.png");
    image[2].object_x[8]     = 750;
    image[2].object_y[8]     = 413;
    image[2].object_type[8]  = OBJECT_TYPE_STATIC;
    image[2].object[9]       = IMG_Load("./image/stage3/stage3_koumori.png");
    image[2].object_x[9]     = 350;
    image[2].object_y[9]     = 200;
    image[2].object_type[9]  = OBJECT_TYPE_STATIC;
    image[2].object[10]      = IMG_Load("./image/stage3/stage3_pumpkin.png");
    image[2].object_x[10]    = 850;
    image[2].object_y[10]    = 375;
    image[2].object_type[10] = OBJECT_TYPE_STATIC;
    image[2].object_num      = 11;

    /* ステージ４の画像読み込み */
    image[3].background = IMG_Load("./image/none/bg.png");

    image[3].object[0]      = IMG_Load("./image/stage4/stage4_tree.png");
    image[3].object_x[0]    = 20;
    image[3].object_y[0]    = 151;
    image[3].object_type[0] = OBJECT_TYPE_STATIC;
    image[3].object[1]      = IMG_Load("./image/stage4/stage4_yukidaruma.png");
    image[3].object_x[1]    = 700;
    image[3].object_y[1]    = 313;
    image[3].object_type[1] = OBJECT_TYPE_STATIC;
    image[3].object[2]      = IMG_Load("./image/none/cloud1.png");
    image[3].object_x[2]    = 63;
    image[3].object_y[2]    = 13;
    image[3].object_type[2] = OBJECT_TYPE_CLOUD;
    image[3].object[3]      = IMG_Load("./image/none/cloud1.png");
    image[3].object_x[3]    = 294;
    image[3].object_y[3]    = 32;
    image[3].object_type[3] = OBJECT_TYPE_CLOUD;
    image[3].object[4]      = IMG_Load("./image/none/cloud1.png");
    image[3].object_x[4]    = 645;
    image[3].object_y[4]    = 17;
    image[3].object_type[4] = OBJECT_TYPE_CLOUD;

    image[3].object_num = 5;

    image_menu = IMG_Load("./image/menu_bg.png"); // メニュー画面の背景

    /* 透過画像の読み込み */
    image_rect[0] = IMG_Load("./image/transparent/rect_bg1.png");
    image_rect[1] = IMG_Load("./image/transparent/rect_bg2.png");

    /* 的の画像読み込み */
    image_target[0] = IMG_Load("./image/target/100-1.png");
    image_target[1] = IMG_Load("./image/target/200-1.png");
    image_target[2] = IMG_Load("./image/target/500-1.png");
    image_target[3] = IMG_Load("./image/target/1000-1.png");
    image_target[4] = IMG_Load("./image/target/2000-1.png");

    image_target_anime[0].frame[0] = IMG_Load("./image/target_effect/100/100-0.png");
    image_target_anime[0].frame[1] = IMG_Load("./image/target_effect/100/100-1.png");
    image_target_anime[0].frame[2] = IMG_Load("./image/target_effect/100/100-2.png");
    image_target_anime[0].frame[3] = IMG_Load("./image/target_effect/100/100-3.png");
    image_target_anime[0].frame[4] = IMG_Load("./image/target_effect/100/100-4.png");
    image_target_anime[0].frame[5] = IMG_Load("./image/target_effect/100/100-5.png");
    image_target_anime[0].frame[6] = IMG_Load("./image/target_effect/100/100-6.png");
    image_target_anime[0].frame[7] = IMG_Load("./image/target_effect/100/100-7.png");
    image_target_anime[0].frame[8] = IMG_Load("./image/target_effect/100/100-8.png");

    image_target_anime[1].frame[0] = IMG_Load("./image/target_effect/200/200-0.png");
    image_target_anime[1].frame[1] = IMG_Load("./image/target_effect/200/200-1.png");
    image_target_anime[1].frame[2] = IMG_Load("./image/target_effect/200/200-2.png");
    image_target_anime[1].frame[3] = IMG_Load("./image/target_effect/200/200-3.png");
    image_target_anime[1].frame[4] = IMG_Load("./image/target_effect/200/200-4.png");
    image_target_anime[1].frame[5] = IMG_Load("./image/target_effect/200/200-5.png");
    image_target_anime[1].frame[6] = IMG_Load("./image/target_effect/200/200-6.png");
    image_target_anime[1].frame[7] = IMG_Load("./image/target_effect/200/200-7.png");
    image_target_anime[1].frame[8] = IMG_Load("./image/target_effect/200/200-8.png");

    image_target_anime[2].frame[0] = IMG_Load("./image/target_effect/500/500-0.png");
    image_target_anime[2].frame[1] = IMG_Load("./image/target_effect/500/500-1.png");
    image_target_anime[2].frame[2] = IMG_Load("./image/target_effect/500/500-2.png");
    image_target_anime[2].frame[3] = IMG_Load("./image/target_effect/500/500-3.png");
    image_target_anime[2].frame[4] = IMG_Load("./image/target_effect/500/500-4.png");
    image_target_anime[2].frame[5] = IMG_Load("./image/target_effect/500/500-5.png");
    image_target_anime[2].frame[6] = IMG_Load("./image/target_effect/500/500-6.png");
    image_target_anime[2].frame[7] = IMG_Load("./image/target_effect/500/500-7.png");
    image_target_anime[2].frame[8] = IMG_Load("./image/target_effect/500/500-8.png");

    image_target_anime[3].frame[0] = IMG_Load("./image/target_effect/1000/1000-0.png");
    image_target_anime[3].frame[1] = IMG_Load("./image/target_effect/1000/1000-1.png");
    image_target_anime[3].frame[2] = IMG_Load("./image/target_effect/1000/1000-2.png");
    image_target_anime[3].frame[3] = IMG_Load("./image/target_effect/1000/1000-3.png");
    image_target_anime[3].frame[4] = IMG_Load("./image/target_effect/1000/1000-4.png");
    image_target_anime[3].frame[5] = IMG_Load("./image/target_effect/1000/1000-5.png");
    image_target_anime[3].frame[6] = IMG_Load("./image/target_effect/1000/1000-6.png");
    image_target_anime[3].frame[7] = IMG_Load("./image/target_effect/1000/1000-7.png");
    image_target_anime[3].frame[8] = IMG_Load("./image/target_effect/1000/1000-8.png");

    image_target_anime[4].frame[0] = IMG_Load("./image/target_effect/2000/2000-0.png");
    image_target_anime[4].frame[1] = IMG_Load("./image/target_effect/2000/2000-1.png");
    image_target_anime[4].frame[2] = IMG_Load("./image/target_effect/2000/2000-2.png");
    image_target_anime[4].frame[3] = IMG_Load("./image/target_effect/2000/2000-3.png");
    image_target_anime[4].frame[4] = IMG_Load("./image/target_effect/2000/2000-4.png");
    image_target_anime[4].frame[5] = IMG_Load("./image/target_effect/2000/2000-5.png");
    image_target_anime[4].frame[6] = IMG_Load("./image/target_effect/2000/2000-6.png");
    image_target_anime[4].frame[7] = IMG_Load("./image/target_effect/2000/2000-7.png");
    image_target_anime[4].frame[8] = IMG_Load("./image/target_effect/2000/2000-8.png");

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
    /* Wiiリモコンの接続を解除する */
    if (wiimote_is_open(&wiimote)) {
        SystemLog("Wiiリモコンの接続を解除します");
        wiimote_disconnect(&wiimote); // Wiiリモコン接続解除
    }

    /* 音楽関係を開放する */
    Mix_FreeMusic(bgm_menu);
    Mix_CloseAudio();

    /* 各スレッドが終了するまでmain関数の処理を中断 */
    SystemLog("各スレッドの終了待ち");
    SDL_WaitThread(keyboard_thread, NULL);   // keyboard_threadの処理終了を待つ
    SDL_WaitThread(wiimote_thread, NULL);    // wiimote_threadの処理終了を待つ
    SDL_WaitThread(wiimote_ir_thread, NULL); // wiimote_ir_threadの処理終了を待つ

    /* タイマーを除去 */
    SystemLog("タイマーを除去します");
    SDL_RemoveTimer(timer_id_countdown);
    SDL_RemoveTimer(timer_id_transition_stage);

    /* サーフェイスを開放 */
    SystemLog("サーフェイスを開放します");
    for (int i = 0; i < IMAGE_BG_NUM; i++)
        SDL_FreeSurface(image_bg[i]);
    for (int i = 0; i < IMAGE_TARGET_NUM; i++)
        SDL_FreeSurface(image_target[i]);
    SDL_FreeSurface(image_menu);
    SDL_FreeSurface(image_rect[0]);
    SDL_FreeSurface(image_rect[1]);

    SystemLog("レンダラーとウィンドウを破棄します");
    SDL_DestroyRenderer(renderer); // RC の破棄（解放）
    SDL_DestroyWindow(window);     // 生成したウィンドウの破棄（消去）
    SDL_Quit();                    // SDL を終了する

    /* フォントをクローズ */
    SystemLog("フォントを開放します");
    TTF_CloseFont(fonts.size10);
    TTF_CloseFont(fonts.size15);
    TTF_CloseFont(fonts.size20);
    TTF_CloseFont(fonts.size25);
    TTF_CloseFont(fonts.size50);
    TTF_Quit(); // TTF を終了する

    exit(EXIT_SUCCESS); // ゲームを終了する
}