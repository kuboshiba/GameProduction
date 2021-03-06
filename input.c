#include "header/define.h"

void wiimote_func__menu();                         // Wiiリモコン入力用の関数　メニュー画面
void wiimote_func__solo_ok_cancel();               // Wiiリモコン入力用の関数　ソロプレイするかどうか
void wiimote_func__input_name();                   // Wiiリモコン入力用の関数　プレイヤー入力
void wiimote_func__result();                       // Wiiリモコン入力用の関数　リザルト画面
void wiimote_func__solo_playing();                 // Wiiリモコン入力用の関数　ソロプレイ中
void wiimote_func__setting();                      // Wiiリモコン入力用の関数　設定画面
void wiimote_func__multi_host_or_client();         // Wiiリモコン入力用の関数　ホストかクライアントか決定
void wiimote_func__multi_host_player_num_decide(); // Wiiリモコン入力用の関数　プレイヤーの数入力
void wiimote_func__multi_client_input_name();      // Wiiリモコン入力用の関数　プレイヤー名の入力
void wiimote_func__multi_playing();                // Wiiリモコン入力用の関数　マルチプレイ中

/*******************************************************************
 * 関数名 : keyboard_func
 * 　　型 : int
 * 　説明 : キーボード入力用の関数
 ******************************************************************/
int keyboard_func()
{
    /* ゲームがアクティブ状態であればループ */
    while (wiimote_is_open(&wiimote)) {
        /* イベントを更新する */
        if (SDL_PollEvent(&event)) {
            /* キーボードの入力タイプによって条件分岐*/
            if (event.type == SDL_KEYDOWN) { // キーが押されたとき
                /* キーの種類によって条件分岐 */
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: // Escキーが押された時
                    /* フラグを全て false にする */
                    for (int i = 0; i < MODE_NUM; i++)
                        flag[i] = false;
                    wiimote_disconnect(&wiimote); // Wiiリモコンの接続を解除する
                    break;
                }
            }
        }
    }
    return 0;
}

/*******************************************************************
 * 関数名 : wiimote_func
 * 　　型 : int
 * 　説明 : Wiiリモコン入力用の関数
 ******************************************************************/
int wiimote_func()
{
    /* Wiiリモコンの状態を取得・更新する */
    while (wiimote_is_open(&wiimote)) {
        wiimote_update(&wiimote);

        /* モードによって条件分岐*/
        switch (gGame.mode) {
        case MODE_MENU: // メニュー
            wiimote_func__menu();
            break;
        case MODE_SOLO_OK_OR_CANCEL: // ソロプレイをするかどうか OK or CANCEL
            wiimote_func__solo_ok_cancel();
            break;
        case MODE_INPUT_NAME: // プレイヤー名を入力する
            wiimote_func__input_name();
            break;
        case MODE_SOLO_PLAYING: // ソロプレイ　プレイ中
            wiimote_func__solo_playing();
            break;
        case MODE_RESULT: // リザルトを表示する
            wiimote_func__result();
            break;
        case MODE_SETTING: // セッティング画面を描画する
            wiimote_func__setting();
            break;
        case MODE_MULTI_HOST_OR_CLIENT: // ホストかクライアントを選択する
            wiimote_func__multi_host_or_client();
            break;
        case MODE_MULTI_HOST_PLAYER_NUM_DECIDE: // プレイヤーの人数を選択する
            wiimote_func__multi_host_player_num_decide();
            break;
        case MODE_MULTI_CLIENT_INPUT_NAME:
            wiimote_func__multi_client_input_name();
            break;
        case MODE_MULTI_PLAYING: // マルチプレイ　プレイ中
            wiimote_func__multi_playing();
            break;
        case MODE_MULTI_PLAYING_WAIT: // マルチプレイ中　同期中
            break;
        case MODE_MULTI_CLIENT_SETUP: // クライアントのセットアップ
            break;
        case MODE_MULTI_CLIENT_WAIT: // クライアントの接続待機中
            break;
        case MODE_MULTI_HOST_SERVER_SETUP: // サーバーをセットアップする
            break;
        case MODE_COUNTDOWN: // カウントダウン時
            break;
        case MODE_TRANSITION: // 画面遷移のアニメーション時
            break;
        }
    }
    return 0;
}

/*******************************************************************
 * 関数名 : wiimote_ir_func
 * 　　型 : int
 * 　説明 : Wiiリモコンの赤外線センサの取得とポインター生成関数
 ******************************************************************/
int wiimote_ir_func()
{
    /* Wiiリモコンがオープン（接続状態）であればループ */
    while (wiimote_is_open(&wiimote)) {
        wiimote_update(&wiimote);

        /* 赤外線センサの値を連続にする（x座標）*/
        if (512 <= wiimote.ir2.x && wiimote.ir2.x <= 767)
            wiimote.ir2.x -= 256;
        else if (1024 <= wiimote.ir2.x && wiimote.ir2.x <= 1279)
            wiimote.ir2.x -= 512;
        else if (1536 <= wiimote.ir2.x && wiimote.ir2.x <= 1791)
            wiimote.ir2.x -= 768;

        wiimote.ir2.x = abs(1023 - wiimote.ir2.x); // x値反転

        /* 赤外線センサの値を連続にする（y座標）*/
        if (512 <= wiimote.ir2.y && wiimote.ir2.y <= 767)
            wiimote.ir2.y -= 256;
        else if (1024 <= wiimote.ir2.y && wiimote.ir2.y <= 1279)
            wiimote.ir2.y -= 512;
        else if (1536 <= wiimote.ir2.y && wiimote.ir2.y <= 1791)
            wiimote.ir2.y -= 768;

        /* map関数でウィンドウサイズに調整 */
        pointer.x = map(wiimote.ir2.x, 100, 900, -50, 1050);
        pointer.y = map(wiimote.ir2.y, 200, 700, -50, 550);

        /* ノイズ除去 */
        if ((abs(pointer.x - pointer_prev.x) <= 5))
            pointer.x = pointer_prev.x;
        if (abs(pointer.y - pointer_prev.y) <= 5)
            pointer.y = pointer_prev.y;

        /* 前回の座標として記憶 */
        pointer_prev.x = pointer.x;
        pointer_prev.y = pointer.y;
    }
    return 0;
}

/*******************************************************************
 * 関数名 : wiimote_func__menu
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　メニュー画面
 ******************************************************************/
void wiimote_func__menu()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (selecter != 0)
            selecter--;
        else
            selecter = 3;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (selecter != 3)
            selecter++;
        else
            selecter = 0;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタンが押されたとき */
    else if (wiimote.keys.a) {
        /*
            セレクター値によって条件分岐
            0: SOLO ソロプレイ >>> ソロプレイをするかを尋ねる
            1: MULTI マルチプレイ >>> ホストかクライアントを尋ねる
            2: SETTING 設定
            3: EXIT 終了
        */
        switch (selecter) {
        case 0: // SOLO ソロプレイ
            gGame.mode = MODE_SOLO_OK_OR_CANCEL;
            break;
        case 1: // MULTI マルチプレイ
            gGame.mode = MODE_MULTI_HOST_OR_CLIENT;
            break;
        case 2: // SETTING 設定
            gGame.mode = MODE_SETTING;
            break;
        case 3: // EXIT 終了
            /* フラグを全て false にする */
            for (int i = 0; i < MODE_NUM; i++)
                flag[i] = false;

            wiimote_disconnect(&wiimote); // Wiiリモコンの接続を解除する
            return;
        }

        selecter = 0; // セレクター初期化

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__solo_ok_cancel
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　ソロプレイするかどうか
 ******************************************************************/
void wiimote_func__solo_ok_cancel()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (selecter != 0)
            selecter--;
        else
            selecter = 1;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (selecter != 1)
            selecter++;
        else
            selecter = 0;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタンが押されたとき */
    else if (wiimote.keys.a) {
        /*
            セレクター値によって条件分岐
            0: OK ソロプレイする >>> プレイヤー名入力へ
            1: CANCEL >>> メニュー画面へ
        */
        switch (selecter) {
        case 0: // OK >>> プレイヤー名入力へ
            gGame.mode = MODE_INPUT_NAME;
            break;
        case 1: // CANCEL >>> メニュー画面へ
            gGame.mode = MODE_MENU;
            break;
        }

        flag[MODE_SOLO_OK_OR_CANCEL] = false;
        selecter                     = 0;

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__input_name
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　プレイヤー入力
 ******************************************************************/
void wiimote_func__input_name()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (9 <= key_pos && key_pos <= 25)
            key_pos -= 9;
        else if (key_pos == 8)
            key_pos = 25;
        else
            key_pos += 18;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (0 <= key_pos && key_pos <= 16)
            key_pos += 9;
        else if (key_pos == 17)
            key_pos = 25;
        else
            key_pos -= 18;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー左 が押されたとき */
    else if (wiimote.keys.left) {
        if (key_pos == 0 || key_pos == 9)
            key_pos += 8;
        else if (key_pos == 18)
            key_pos = 25;
        else
            key_pos--;

        /* チャタリング防止 */
        while (wiimote.keys.left)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー右 が押されたとき */
    else if (wiimote.keys.right) {
        if (key_pos == 8 || key_pos == 17)
            key_pos -= 8;
        else if (key_pos == 25)
            key_pos = 18;
        else
            key_pos++;

        /* チャタリング防止 */
        while (wiimote.keys.right)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタン が押されたとき */
    else if (wiimote.keys.a) {
        char buf[MAX_LEN_NAME] = "\0";
        if (strlen(gPlayer.name) <= 15) {
            sprintf(buf, "%s%s", gPlayer.name, alphabet[key_pos]);
            sprintf(gPlayer.name, "%s", buf);
        }

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Bボタン が押されたとき */
    else if (wiimote.keys.b) {
        int len = strlen(gPlayer.name);
        if (0 < len)
            gPlayer.name[len - 1] = '\0';

        /* チャタリング防止 */
        while (wiimote.keys.b)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 1ボタン が押されたとき */
    else if (wiimote.keys.one) {
        gGame.mode            = MODE_SOLO_PLAYING; // モード遷移
        flag[MODE_INPUT_NAME] = false;             // フラグ初期化
        key_pos               = 0;                 // セレクター初期化

        /* チャタリング防止 */
        while (wiimote.keys.one)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__result
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　リザルト画面
 ******************************************************************/
void wiimote_func__result()
{
    /* Wiiリモコンの 十字キー左 が押されたとき */
    if (wiimote.keys.left) {
        if (selecter != 0)
            selecter--;

        /* チャタリング防止 */
        while (wiimote.keys.left)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー右 が押されたとき */
    else if (wiimote.keys.right) {
        if (selecter != 1)
            selecter++;

        /* チャタリング防止 */
        while (wiimote.keys.right)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタン が押されたとき */
    else if (wiimote.keys.a) {
        switch (selecter) {
        case 0: // OK
            flag[MODE_RESULT] = false;
            gGame.mode        = MODE_MENU;
            break;
        case 1: // REPLAY
            flag[MODE_RESULT] = false;
            gGame.mode        = MODE_SOLO_PLAYING;
            break;
        }
        selecter = 0; // セレクター初期化

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__solo_playing
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　ソロプレイ中
 ******************************************************************/
void wiimote_func__solo_playing()
{
    /* 
       Wiiリモコンの Bボタン が押されたとき
       的との当たり判定を実行する
    */
    if (wiimote.keys.b) {
        for (int i = 0; i < TARGET_NUM_MAX; i++) {
            /* 的を表示している場合　ポインターと的の当たり判定をする */
            if (s_data.target[i].type != 5) {
                int a = (s_data.target[i].x + 25) - pointer.x;
                int b = (s_data.target[i].y + 25) - pointer.y;
                int c = sqrt(a * a + b * b);

                /* 当たっている場合 */
                if (c <= 34) {
                    switch (s_data.target[i].type) {
                    case 0: // 100点
                        gPlayer.score += 100;
                        break;
                    case 1: // 200点
                        gPlayer.score += 200;
                        break;
                    case 2: // 500点
                        gPlayer.score += 500;
                        break;
                    case 3: // 1000点
                        gPlayer.score += 1000;
                        break;
                    case 4: // 2000点
                        gPlayer.score += 2000;
                        break;
                    }
                    s_data.target[i].type = 5; // 的を消す
                    s_data.target[i].cnt  = 0; // カウンターを初期化
                }
            }
        }

        /* チャタリング防止 */
        while (wiimote.keys.b)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__setting
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　設定画面
 ******************************************************************/
void wiimote_func__setting()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (selecter != 0)
            selecter--;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (selecter != 1)
            selecter++;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー左 が押されたとき */
    else if (wiimote.keys.left) {
        /* selecter によって条件分岐
            音量を下げる
            0: MUSIC
            1: 
        */
        switch (selecter) {
        case 0: // MUSIC
            /* 音量を下げる */
            if (0 <= music_volume - 4) {
                music_volume -= 4;
                Mix_VolumeMusic(music_volume);
                SDL_Delay(100);
            }
            break;
        case 1:
            break;
        }

        /* チャタリング防止 */
        // while (wiimote.keys.left)
        //     if (wiimote_is_open(&wiimote))
        //         wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー右 が押されたとき */
    else if (wiimote.keys.right) {
        /* selecter によって条件分岐
            音量を挙げる
            0: MUSIC
            1: 
        */
        switch (selecter) {
        case 0: // MUSIC
            if (music_volume + 4 <= 128) {
                music_volume += 4;
                Mix_VolumeMusic(music_volume);
                SDL_Delay(100);
            }
            break;
        case 1:
            break;
        }

        /* チャタリング防止 */
        // while (wiimote.keys.right)
        //     if (wiimote_is_open(&wiimote))
        //         wiimote_update(&wiimote);
    }
    /* Wiiリモコンの １ボタン が押されたとき */
    else if (wiimote.keys.one) {
        selecter           = 0; // セレクター初期化
        flag[MODE_SETTING] = false;
        gGame.mode         = MODE_MENU;

        /* チャタリング防止 */
        while (wiimote.keys.b)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__multi_host_or_client
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　ホストかクライアントを選択
 ******************************************************************/
void wiimote_func__multi_host_or_client()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (selecter != 0)
            selecter--;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (selecter != 2)
            selecter++;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタン が押されたとき */
    else if (wiimote.keys.a) {

        switch (selecter) {
        case 0: // HOST
            gGame.mode = MODE_MULTI_HOST_PLAYER_NUM_DECIDE;
            break;
        case 1: // CLIENT
            gGame.mode = MODE_MULTI_CLIENT_INPUT_NAME;
            break;
        case 2: // RETURN MENU
            gGame.mode = MODE_MENU;
            break;
        }

        selecter = 0;

        flag[MODE_MULTI_HOST_OR_CLIENT] = false;

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__multi_host_player_num_decide
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　プレイヤーの人数を選択
 ******************************************************************/
void wiimote_func__multi_host_player_num_decide()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (selecter != 0)
            selecter--;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (selecter != 3)
            selecter++;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタン が押されたとき */
    else if (wiimote.keys.a) {

        switch (selecter) {
        case 0: // 2 players
            gGame.player_num = 2;
            gGame.mode       = MODE_MULTI_HOST_SERVER_SETUP;
            break;
        case 1: // 3 players
            gGame.player_num = 3;
            gGame.mode       = MODE_MULTI_HOST_SERVER_SETUP;
            break;
        case 2: // 4 players
            gGame.player_num = 4;
            gGame.mode       = MODE_MULTI_HOST_SERVER_SETUP;
            break;
        case 3: // RETURN MENU
            gGame.mode = MODE_MENU;
            break;
        }

        selecter = 0;

        flag[MODE_MULTI_HOST_PLAYER_NUM_DECIDE] = false;

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__multi_client_input_name
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　クライアントの名前入力
 ******************************************************************/
void wiimote_func__multi_client_input_name()
{
    /* Wiiリモコンの 十字キー上 が押されたとき */
    if (wiimote.keys.up) {
        if (9 <= key_pos && key_pos <= 25)
            key_pos -= 9;
        else if (key_pos == 8)
            key_pos = 25;
        else
            key_pos += 18;

        /* チャタリング防止 */
        while (wiimote.keys.up)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー下 が押されたとき */
    else if (wiimote.keys.down) {
        if (0 <= key_pos && key_pos <= 16)
            key_pos += 9;
        else if (key_pos == 17)
            key_pos = 25;
        else
            key_pos -= 18;

        /* チャタリング防止 */
        while (wiimote.keys.down)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー左 が押されたとき */
    else if (wiimote.keys.left) {
        if (key_pos == 0 || key_pos == 9)
            key_pos += 8;
        else if (key_pos == 18)
            key_pos = 25;
        else
            key_pos--;

        /* チャタリング防止 */
        while (wiimote.keys.left)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー右 が押されたとき */
    else if (wiimote.keys.right) {
        if (key_pos == 8 || key_pos == 17)
            key_pos -= 8;
        else if (key_pos == 25)
            key_pos = 18;
        else
            key_pos++;

        /* チャタリング防止 */
        while (wiimote.keys.right)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Aボタン が押されたとき */
    else if (wiimote.keys.a) {
        char buf[MAX_LEN_NAME] = "\0";
        if (strlen(gPlayer.name) <= 15) {
            sprintf(buf, "%s%s", gPlayer.name, alphabet[key_pos]);
            sprintf(gPlayer.name, "%s", buf);
        }

        /* チャタリング防止 */
        while (wiimote.keys.a)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Bボタン が押されたとき */
    else if (wiimote.keys.b) {
        int len = strlen(gPlayer.name);
        if (0 < len)
            gPlayer.name[len - 1] = '\0';

        /* チャタリング防止 */
        while (wiimote.keys.b)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 1ボタン が押されたとき */
    else if (wiimote.keys.one) {
        gGame.mode                         = MODE_MULTI_CLIENT_SETUP; // モード遷移
        flag[MODE_MULTI_CLIENT_INPUT_NAME] = false;                   // フラグ初期化
        key_pos                            = 0;                       // セレクター初期化

        /* チャタリング防止 */
        while (wiimote.keys.one)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}

/*******************************************************************
 * 関数名 : wiimote_func__multi_playing
 * 　　型 : void
 * 　説明 : Wiiリモコン入力用の関数　マルチプレイ中
 ******************************************************************/
void wiimote_func__multi_playing()
{
    /* 
       Wiiリモコンの Bボタン が押されたとき
       的との当たり判定を実行する
    */
    if (wiimote.keys.b) {
        for (int i = 0; i < TARGET_NUM_MAX; i++) {
            /* 的を表示している場合　ポインターと的の当たり判定をする */
            if (c_data.target[i].type != 5) {
                int a = (c_data.target[i].x + 25) - pointer.x;
                int b = (c_data.target[i].y + 25) - pointer.y;
                int c = sqrt(a * a + b * b);

                /* 当たっている場合 */
                if (c <= 34) {
                    switch (c_data.target[i].type) {
                    case 0: // 100点
                        gPlayer.score += 100;
                        break;
                    case 1: // 200点
                        gPlayer.score += 200;
                        break;
                    case 2: // 500点
                        gPlayer.score += 500;
                        break;
                    case 3: // 1000点
                        gPlayer.score += 1000;
                        break;
                    case 4: // 2000点
                        gPlayer.score += 2000;
                        break;
                    }
                    c_data.target[i].type_buf = c_data.target[i].type;
                    c_data.target[i].type     = 6; // 的を消す
                    c_data.target[i].cnt      = 0; // カウンターを初期化

                    c_data.target[i].c_myid = c_myid;

                    c_data.score[c_myid] = gPlayer.score;
                    c_data.command       = C_TO_S_TARGET_COMMAND;
                    client_send_data(&c_data, sizeof(c_data));

                    timer_id_target_animation[i] = SDL_AddTimer(100, animation_target_func, NULL);

                    break;
                }
            }
        }

        /* チャタリング防止 */
        while (wiimote.keys.b)
            if (wiimote_is_open(&wiimote))
                wiimote_update(&wiimote);
    }
}
