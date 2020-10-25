#include "header/define.h"

// Wiiリモコンの入力制御関数
int wii_func(void *args)
{
    SDL_mutex *mtx = (SDL_mutex *)args; // 引数型はmtxに変更

    // Wiiリモコンがオープン（接続状態）であればループ
    while (wiimote_is_open(&wiimote)) {
        // Wiiリモコンの状態を取得・更新する
        if (wiimote_update(&wiimote)) {
            SDL_LockMutex(mtx); // Mutexをロックして、他のスレッドが共有変数にアクセスできないようにする
            // Wii Homeボタンが押された時
            if (wiimote.keys.home)
                wiimote_disconnect(&wiimote); // Wiiリモコンとの接続を解除

            // メニュー画面で選択されているモードで条件分岐
            switch (gGame.mode) {
            // メニュー
            case MD_MENU:
                // Wiiリモコンの 十字キー上 が押されたとき
                if (wiimote.keys.up) {
                    // メニューのモードをデクリメント
                    if (menu_mode != 0)
                        menu_mode--;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // Wiiリモコンの 十字キー下 が押されたとき
                else if (wiimote.keys.down) {
                    // メニューのモードをインクリメント
                    if (menu_mode != 2)
                        menu_mode++;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.down)
                        wiimote_update(&wiimote);
                }
                // Wiiリモコンの Aボタン が押されたとき
                else if (wiimote.keys.a) {
                    // メニューのモードによって条件分岐
                    // 0: ソロプレイ，1: マルチプレイ, 2: 設定
                    switch (menu_mode) {
                    case 0: // ソロプレイのボタンが押されたとき
                        player_num      = 1;
                        gGame.mode      = MD_SOLO_WAIT;
                        gPlayer[0].mode = MD_SOLO_WAIT;
                        break;
                    case 1: // マルチプレイのボタンが押されたとき
                        gGame.mode      = MD_MULTI_WAIT;
                        gPlayer[0].mode = MD_MULTI_WAIT;
                        break;
                    case 2: // 設定のボタンが押されたとき
                        break;
                    default:
                        break;
                    }
                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.a)
                        wiimote_update(&wiimote);
                }
                break;
            // ソロプレイの待機
            case MD_SOLO_WAIT:
                // 十字キー上が押されたとき
                if (wiimote.keys.up) {
                    // セレクターをデクリメント
                    if (menu_sel != 0)
                        menu_sel--;
                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // 十字キー下が押されたとき
                else if (wiimote.keys.down) {
                    // セレクターをインクリメント
                    if (menu_sel != 1)
                        menu_sel++;
                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.down)
                        wiimote_update(&wiimote);
                }
                // Aボタンが押されたとき
                else if (wiimote.keys.a) {
                    // セレクターによって条件分岐
                    switch (menu_sel) {
                    // OKボタンが押されたとき
                    case SEL_OK:
                        player_num      = 1;               // プレイヤーの数を１に設定
                        gGame.mode      = MD_SOLO_PLAYING; // モードをソロプレイに設定
                        gPlayer[0].mode = MD_SOLO_PLAYING; // モードをソロプレイに設定
                        break;
                    // CANCELボタンが押されたとき
                    case SEL_CANCEL:
                        player_num      = 1;       // プレイヤーの数取り敢えず１に初期化
                        gGame.mode      = MD_MENU; // モードをメニューに設定
                        gPlayer[0].mode = MD_MENU; // モードをメニューに設定
                        break;
                    default:
                        break;
                    }
                    menu_sel = SEL_OK; // セレクターを初期化

                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.a)
                        wiimote_update(&wiimote);
                }
                break;
            // マルチプレイの待機
            case MD_MULTI_WAIT:
                // 十字キー上が押されたとき
                if (wiimote.keys.up) {
                    // セレクターをデクリメント
                    if (menu_sel != 0)
                        menu_sel--;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // 十字キー下が押されたとき
                else if (wiimote.keys.down) {
                    // セレクターをインクリメント
                    if (menu_sel != 2)
                        menu_sel++;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.down)
                        wiimote_update(&wiimote);
                }
                // Aボタンが押されたとき
                else if (wiimote.keys.a) {
                    // セレクターによって条件分岐
                    switch (menu_sel) {
                    case SEL_HOST:
                        puts("host");
                        break;
                    case SEL_CLIENT:
                        puts("client");
                        break;
                    case 2:
                        puts("cancel");
                        break;
                    default:
                        break;
                    }
                }
                break;
            default:
                break;
            }
            SDL_UnlockMutex(mtx); // Mutexをアンロックし、他のスレッドが共有変数にアクセスできるようにする
        } else {
            wiimote_disconnect(&wiimote);
        }
    }
    return 0;
}

// キーボードの入力制御関数
int keyboard_func(void *args)
{
    SDL_mutex *mtx = (SDL_mutex *)args; // 注意：引数はmtx

    // Wiiリモコンがオープン（接続状態）であればループ
    while (wiimote_is_open(&wiimote)) {
        // キーボードの状態を取得・更新する
        if (SDL_PollEvent(&event)) {
            SDL_LockMutex(mtx); // Mutexをロックして、他のスレッドが共有変数にアクセスできないようにする

            switch (event.type) {
            case SDL_KEYDOWN: // キーボードが押された時
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: // Escキーが押された時
                    wiimote_disconnect(&wiimote);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }

            SDL_UnlockMutex(mtx); // Mutexをアンロックし、他のスレッドが共有変数にアクセスできるようにする
        }
    }
    return 0;
}