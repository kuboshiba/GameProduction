#include "header/define.h"

void wiimote_func__menu();           // Wiiリモコン入力用の関数　メニュー画面
void wiimote_func__solo_ok_cancel(); // Wiiリモコン入力用の関数　ソロプレイするかどうか
void wiimote_func__input_name();     // Wiiリモコン入力用の関数　プレイヤー入力
void wiimote_func__result();         // Wiiリモコン入力用の関数　リザルト画面
void wiimote_func__solo_playing();   // Wiiリモコン入力用の関数　ソロプレイ中

/*******************************************************************
 * 関数名 : keyboard_func
 * 　　型 : int
 * 　説明 : キーボード入力用の関数
 ******************************************************************/
int keyboard_func()
{
    /* ゲームがアクティブ状態であればループ */
    while (gGame.status == ACTIVE) {
        /* イベントを更新する */
        if (SDL_PollEvent(&event)) {
            /* キーボードの入力タイプによって条件分岐*/
            if (event.type == SDL_KEYDOWN) { // キーが押されたとき
                /* キーの種類によって条件分岐 */
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:           // Escキーが押された時
                    gGame.status = PASSIVE; // ゲームの状態を PASSIVE にする
                    /* フラグを全て false にする */
                    for (int i = 0; i < MODE_NUM; i++)
                        flag[i] = false;
                    return;
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
    /* ゲームがアクティブ状態であればループ */
    while (gGame.status == ACTIVE) {
        /* Wiiリモコンの状態を取得・更新する */
        if (wiimote_update(&wiimote)) {
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
            case MODE_RESULT:
                wiimote_func__result();
                break;
            case MODE_SOLO_REPLAY: // ソロプレイ　リプレイ
                wiimote_func__solo_playing();
                break;
            case MODE_COUNTDOWN: // カウントダウン時
                break;
            case MODE_TRANSITION: // 画面遷移のアニメーション時
                break;
            }
        }
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
            break;
        case 2: // SETTING 設定
            break;
        case 3:                     // EXIT 終了
            gGame.status = PASSIVE; // ゲームの状態を PASSIVE にする
            /* フラグを全て false にする */
            for (int i = 0; i < MODE_NUM; i++)
                flag[i] = false;
            return;
        }

        /* チャタリング防止 */
        while (wiimote.keys.a)
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
            wiimote_update(&wiimote);
    }
    /* Wiiリモコンの Bボタン が押されたとき */
    else if (wiimote.keys.b) {
        int len = strlen(gPlayer.name);
        if (0 < len)
            gPlayer.name[len - 1] = '\0';

        /* チャタリング防止 */
        while (wiimote.keys.b)
            wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 1ボタン が押されたとき */
    else if (wiimote.keys.one) {
        gGame.mode            = MODE_SOLO_PLAYING; // モード遷移
        flag[MODE_INPUT_NAME] = false;             // フラグ初期化
        key_pos               = 0;                 // セレクター初期化

        /* チャタリング防止 */
        while (wiimote.keys.one)
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
            wiimote_update(&wiimote);
    }
    /* Wiiリモコンの 十字キー右 が押されたとき */
    else if (wiimote.keys.right) {
        if (selecter != 1)
            selecter++;

        /* チャタリング防止 */
        while (wiimote.keys.right)
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
            gGame.mode        = MODE_SOLO_REPLAY;
            break;
        }
        selecter = 0; // セレクター初期化

        /* チャタリング防止 */
        while (wiimote.keys.a)
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
}
