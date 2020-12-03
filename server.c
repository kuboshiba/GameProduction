#include "header/define.h"

// サーバー処理のメイン関数
int server_main()
{
    // int num_cl   = player_num;   // クライアントの数をここで決める
    // u_short port = DEFAULT_PORT; // ポート番号はデフォルト　constants.h 参照

    // setup_server(num_cl, port); // サーバーのセットアップを行う

    // メインループ
    // int cond = 1;
    // while (cond) {
    //     cond = control_requests();
    // }

    // terminate_server(); // サーバーの終了処理を行う

    return 0;
}

// サーバーのセットアップを行う関数
void setup_server(int num_cl, u_short port)
{
    int rsock, sock = 0;
    struct sockaddr_in sv_addr, cl_addr;

    fprintf(stderr, "Server setup is started.\n");

    num_clients = num_cl; // クライアントの数を同期

    rsock = socket(AF_INET, SOCK_STREAM, 0); // ソケットのトークンを取得する

    // ソケットのトークンが取得できなかった場合，例外処理
    if (rsock < 0) {
        handle_error("socket()");
    }
    fprintf(stderr, "sock() for request socket is done successfully.\n");

    sv_addr.sin_family      = AF_INET;     // アドレスの種類 = インターネット
    sv_addr.sin_port        = htons(port); // ポート番号 = port
    sv_addr.sin_addr.s_addr = INADDR_ANY;  // 任意アドレスから受信可

    int opt = 1;
    setsockopt(rsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // ソケットオプションをセット

    // ソケットに設定を結び付ける
    if (bind(rsock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
        handle_error("bind()");
    }
    fprintf(stderr, "bind() is done successfully.\n");

    // 接続ソケットの準備
    if (listen(rsock, num_clients) != 0) {
        handle_error("listen()");
    }
    fprintf(stderr, "listen() is started.\n");

    int i, max_sock = 0;
    socklen_t len;
    char src[MAX_LEN_ADDR];

    // クライアントからサーバーの接続要求を処理する
    for (i = 0; i < num_clients; i++) {
        len  = sizeof(cl_addr);
        sock = accept(rsock, (struct sockaddr *)&cl_addr, &len);
        if (sock < 0) {
            handle_error("accept()");
        }
        if (max_sock < sock) {
            max_sock = sock;
        }
        if (read(sock, clients[i].name, MAX_LEN_NAME) == -1) {
            handle_error("read()");
        }
        clients[i].cid  = i;
        clients[i].sock = sock;
        clients[i].addr = cl_addr;
        memset(src, 0, sizeof(src));
        inet_ntop(AF_INET, (struct sockaddr *)&cl_addr.sin_addr, src, sizeof(src));
        fprintf(stderr, "Client %d is accepted (name=%s, address=%s, port=%d).\n", i, clients[i].name, src, ntohs(cl_addr.sin_port));
    }

    close(rsock);

    int j;
    for (i = 0; i < num_clients; i++) {
        send_data(i, &num_clients, sizeof(int));
        send_data(i, &i, sizeof(int));
        for (j = 0; j < num_clients; j++) {
            send_data(i, &clients[j], sizeof(CLIENT));
        }
    }

    num_socks = max_sock + 1;
    FD_ZERO(&mask);   // maskをゼロクリア
    FD_SET(0, &mask); // 0番目のFDに対応する値を1にセット

    // サーバーのセットアップ完了
    for (i = 0; i < num_clients; i++) {
        FD_SET(clients[i].sock, &mask);
    }
    fprintf(stderr, "Server setup is done.\n");

    // data.permittion = permittion; // 発言権を初期化　初めは クライアント０ が発言権所有
    // send_data(BROADCAST, &permittion, sizeof(int));
}

// データ制御を行う関数
int control_requests()
{
    return 0;
}

// データの送信を行う関数
void send_data(int cid, void *data, int size)
{
    if ((cid != BROADCAST) && (0 > cid || cid >= num_clients)) {
        // 異常終了
        fprintf(stderr, "send_data(): client id is illeagal.\n");
        exit(1);
    }
    if ((data == NULL) || (size <= 0)) {
        // 異常終了
        fprintf(stderr, "send_data(): data is illeagal.\n");
        exit(1);
    }

    if (cid == BROADCAST) {
        int i;
        for (i = 0; i < num_clients; i++) {
            if (write(clients[i].sock, data, size) < 0) {
                handle_error("write()");
            }
        }
    } else {
        if (write(clients[cid].sock, data, size) < 0) {
            handle_error("write()");
        }
    }
}

// サーバーの終了処理を行う関数
void terminate_server(void)
{
    int i;
    for (i = 0; i < num_clients; i++) {
        close(clients[i].sock);
    }
    fprintf(stderr, "All connections are closed.\n");
    exit(0);
}

// エラー内容を出力
void handle_error(char *message)
{
    perror(message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}
