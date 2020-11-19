#include "header/define.h"

// Wiiリモコンの入力制御関数
int wii_func(void *args)
{
    SDL_mutex *mtx = (SDL_mutex *)args; // 引数型はmtxに変更

    // Wiiリモコンがオープン（接続状態）であればループ
    while (flag_loop) {
        // Wiiリモコンの状態を取得・更新する
        if (wiimote_update(&wiimote)) {
            SDL_LockMutex(mtx); // Mutexをロックして、他のスレッドが共有変数にアクセスできないようにする
            // Wii Homeボタンが押された時
            if (wiimote.keys.home && gGame.mode != MD_SOLO_PLAYING)
                flag_loop = false;

            // メニュー画面で選択されているモードで条件分岐
            switch (gGame.mode) {
            // [モード] メニュー
            case MD_MENU:
                // Wiiリモコンの 十字キー上 が押されたとき
                if (wiimote.keys.up) {
                    // メニューのモードをデクリメント
                    if (menu_sel != 0)
                        menu_sel--;
                    else
                        menu_sel = 3;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // Wiiリモコンの 十字キー下 が押されたとき
                else if (wiimote.keys.down) {
                    // メニューのモードをインクリメント
                    if (menu_sel != 3)
                        menu_sel++;
                    else
                        menu_sel = 0;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.down)
                        wiimote_update(&wiimote);
                }
                // Wiiリモコンの Aボタン が押されたとき
                else if (wiimote.keys.a) {
                    // メニューのモードによって条件分岐
                    // 0: ソロプレイ，1: マルチプレイ, 2: 設定
                    switch (menu_sel) {
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
                    case 3: // 終了ボタンが押されたとき
                        gGame.mode      = MD_EXIT_WAIT;
                        gPlayer[0].mode = MD_EXIT_WAIT;
                        break;
                    default:
                        break;
                    }
                    menu_sel = 0; // セレクターを初期化

                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.a)
                        wiimote_update(&wiimote);
                }
                break;
            // [モード] ソロプレイの待機
            case MD_SOLO_WAIT:
                // 十字キー上が押されたとき
                if (wiimote.keys.up) {
                    // セレクターをデクリメント
                    if (menu_sel != 0)
                        menu_sel--;
                    else
                        menu_sel = 1;
                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // 十字キー下が押されたとき
                else if (wiimote.keys.down) {
                    // セレクターをインクリメント
                    if (menu_sel != 1)
                        menu_sel++;
                    else
                        menu_sel = 0;
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
                        player_num      = 1;       // プレイヤーの数を取り敢えず１に初期化
                        gGame.mode      = MD_MENU; // モードをメニューに設定
                        gPlayer[0].mode = MD_MENU; // モードをメニューに設定
                        break;
                    default:
                        break;
                    }
                    menu_sel = 0; // セレクターを初期化

                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.a)
                        wiimote_update(&wiimote);
                }
                break;
            case MD_SOLO_PLAYING:
                if (wiimote.keys.home) {
                    flag_playing    = false;
                    player_num      = 1;       // プレイヤーの数を取り敢えず１に初期化
                    gGame.mode      = MD_MENU; // モードをメニューに設定
                    gPlayer[0].mode = MD_MENU; // モードをメニューに設定
                }
                menu_sel = 0; // セレクターを初期化

                // チャタリング防止のための待機用ループ
                while (wiimote.keys.home)
                    wiimote_update(&wiimote);
                break;
            // プレイヤー名を入力するモード
            case MD_PLAYER_NAME_INPUT:
                if (wiimote.keys.right && alpha_key_pos + 1 <= 27) {
                    alpha_key_pos++;
                    goto S1;
                } else if (wiimote.keys.left && alpha_key_pos - 1 >= 0) {
                    alpha_key_pos--;
                    goto S1;
                } else if (wiimote.keys.up && alpha_key_pos - 9 >= 0 && alpha_key_pos < 26) {
                    alpha_key_pos -= 9;
                    goto S1;
                } else if (wiimote.keys.down && alpha_key_pos + 9 < 26 && alpha_key_pos < 18) {
                    alpha_key_pos += 9;
                    goto S1;
                }

                if (wiimote.keys.left && alpha_key_pos == 0)
                    alpha_key_pos = 27;
                else if ((wiimote.keys.right || wiimote.keys.down) && alpha_key_pos == 27)
                    alpha_key_pos = 0;
                else if (wiimote.keys.down && alpha_key_pos == 26)
                    alpha_key_pos = 0;

                if (wiimote.keys.down && 20 <= alpha_key_pos && alpha_key_pos <= 25)
                    alpha_key_pos = 27;
                else if (wiimote.keys.down && 18 <= alpha_key_pos && alpha_key_pos <= 19)
                    alpha_key_pos = 26;
                else if (wiimote.keys.up && alpha_key_pos == 26)
                    alpha_key_pos = 18;
                else if (wiimote.keys.up && alpha_key_pos == 27)
                    alpha_key_pos = 21;

            S1:

                while (wiimote.keys.right || wiimote.keys.left || wiimote.keys.up || wiimote.keys.down)
                    wiimote_update(&wiimote);

                if (wiimote.keys.home) {
                    flag_playing    = false;
                    player_num      = 1;       // プレイヤーの数を取り敢えず１に初期化
                    gGame.mode      = MD_MENU; // モードをメニューに設定
                    gPlayer[0].mode = MD_MENU; // モードをメニューに設定
                }
                menu_sel = 0; // セレクターを初期化

                // チャタリング防止のための待機用ループ
                while (wiimote.keys.home)
                    wiimote_update(&wiimote);
                break;
            // [モード] マルチプレイの待機
            case MD_MULTI_WAIT:
                // 十字キー上が押されたとき
                if (wiimote.keys.up) {
                    // セレクターをデクリメント
                    if (menu_sel != 0)
                        menu_sel--;
                    else
                        menu_sel = 2;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // 十字キー下が押されたとき
                else if (wiimote.keys.down) {
                    // セレクターをインクリメント
                    if (menu_sel != 2)
                        menu_sel++;
                    else
                        menu_sel = 0;
                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.down)
                        wiimote_update(&wiimote);
                }
                // Aボタンが押されたとき
                else if (wiimote.keys.a) {
                    // セレクターによって条件分岐
                    switch (menu_sel) {
                    // ホストが選択されたとき
                    case SEL_HOST:
                        puts("host");
                        break;
                    // クライアントが選択されたとき
                    case SEL_CLIENT:
                        puts("client");
                        break;
                    // キャンセルボタンが選択されたとき
                    case 2:
                        player_num      = 1;       // プレイヤーの数取り敢えず１に初期化
                        gGame.mode      = MD_MENU; // モードをメニューに設定
                        gPlayer[0].mode = MD_MENU; // モードをメニューに設定
                        break;
                    default:
                        break;
                    }
                    menu_sel = 0; // セレクターを初期化

                    // チャタリング防止のための待機用ループ
                    while (wiimote.keys.a)
                        wiimote_update(&wiimote);
                }
                break;
            // [モード] 終了待機
            case MD_EXIT_WAIT:
                // 十字キー上が押されたとき
                if (wiimote.keys.up) {
                    // セレクターをデクリメント
                    if (menu_sel != 0)
                        menu_sel--;
                    else
                        menu_sel = 1;
                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.up)
                        wiimote_update(&wiimote);
                }
                // 十字キー下が押されたとき
                else if (wiimote.keys.down) {
                    // セレクターをインクリメント
                    if (menu_sel != 1)
                        menu_sel++;
                    else
                        menu_sel = 0;
                    // チャタリング防止のため待機用ループ
                    while (wiimote.keys.down)
                        wiimote_update(&wiimote);
                }
                // Aボタンが押されたとき
                else if (wiimote.keys.a) {
                    // セレクターによって条件分岐
                    switch (menu_sel) {
                    // ホストが選択されたとき
                    case 0:
                        gGame.mode      = MD_EXIT;
                        gPlayer[0].mode = MD_EXIT;
                        break;
                    case 1:
                        // チャタリング防止のための待機用ループ
                        while (wiimote.keys.a)
                            wiimote_update(&wiimote);

                        player_num      = 1;       // プレイヤーの数取り敢えず１に初期化
                        gGame.mode      = MD_MENU; // モードをメニューに設定
                        gPlayer[0].mode = MD_MENU; // モードをメニューに設定
                        break;
                    default:
                        break;
                    }
                    menu_sel = 0; // セレクターを初期化
                }
                break;
            // [モード] 終了
            case MD_EXIT:
                // Wiiリモコンが接続状態であればループ
                flag_loop = false;
            default:
                break;
            }

            // 赤外線センサの値を連続にする（x座標）
            if (512 <= wiimote.ir2.x && wiimote.ir2.x <= 767)
                wiimote.ir2.x -= 256;
            else if (1024 <= wiimote.ir2.x && wiimote.ir2.x <= 1279)
                wiimote.ir2.x -= 512;
            else if (1536 <= wiimote.ir2.x && wiimote.ir2.x <= 1791)
                wiimote.ir2.x -= 768;

            wiimote.ir2.x = abs(1023 - wiimote.ir2.x); // x値反転

            // 赤外線センサの値を連続にする（y座標）
            if (512 <= wiimote.ir2.y && wiimote.ir2.y <= 767)
                wiimote.ir2.y -= 256;
            else if (1024 <= wiimote.ir2.y && wiimote.ir2.y <= 1279)
                wiimote.ir2.y -= 512;
            else if (1536 <= wiimote.ir2.y && wiimote.ir2.y <= 1791)
                wiimote.ir2.y -= 768;

            // map関数でウィンドウサイズに調整
            pointer.x = map(wiimote.ir2.x, 100, 900, -50, 1050);
            pointer.y = map(wiimote.ir2.y, 200, 700, -50, 550);

            // ノイズ除去
            if ((abs(pointer.x - pointer_prev.x) <= 5))
                pointer.x = pointer_prev.x;
            if (abs(pointer.y - pointer_prev.y) <= 5)
                pointer.y = pointer_prev.y;

            // 前回の座標として記憶
            pointer_prev.x = pointer.x;
            pointer_prev.y = pointer.y;

            SDL_UnlockMutex(mtx); // Mutexをアンロックし、他のスレッドが共有変数にアクセスできるようにする
        } else {
            flag_loop = false;
        }
    }
    return 0;
}

// キーボードの入力制御関数
int keyboard_func(void *args)
{
    SDL_mutex *mtx = (SDL_mutex *)args; // 注意：引数はmtx

    // Wiiリモコンがオープン（接続状態）であればループ
    while (flag_loop) {
        // キーボードの状態を取得・更新する
        if (SDL_PollEvent(&event)) {
            SDL_LockMutex(mtx); // Mutexをロックして、他のスレッドが共有変数にアクセスできないようにする

            switch (event.type) {
            case SDL_KEYDOWN: // キーボードが押された時
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: // Escキーが押された時
                    flag_loop = false;
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