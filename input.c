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
            if (wiimote.keys.home) {
                wiimote_disconnect(&wiimote); // Wiiリモコンとの接続を解除
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