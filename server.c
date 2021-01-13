#include "header/define.h"

/*******************************************************************
 * 関数名 : server_main
 * 　　型 : int
 * 　説明 : サーバー起動から終了までのメイン処理
 ******************************************************************/
int server_main()
{
    int num_cl   = gGame.player_num; // クライアントの数をここで決める
    u_short port = DEFAULT_PORT;     // ポート番号はデフォルト　constants.h 参照

    setup_server(num_cl, port); // サーバーのセットアップを行う

    // メインループ
    int cond = 1;
    while (cond) {
        cond = server_control_requests();
    }
    terminate_server(); // サーバーの終了処理を行う

    return 0;
}

/*******************************************************************
 * 関数名 : setup_server
 * 　　型 : void
 * 　引数 : num_cl（クライアントの数）, port（ポート番号格納変数）
 * 　説明 : サーバーのセットアップを行う関数
 ******************************************************************/
void setup_server(int num_cl, u_short port)
{
    int rsock, sock = 0;
    struct sockaddr_in sv_addr, cl_addr;

    fprintf(stderr, "Server setup is started.\n");

    s_num_clients = num_cl; // クライアントの数を同期

    rsock = socket(AF_INET, SOCK_STREAM, 0); // ソケットのトークンを取得する
    // ソケットのトークンが取得できなかった場合，例外処理
    if (rsock < 0) {
        server_handle_error("socket()");
    }
    fprintf(stderr, "sock() for request socket is done successfully.\n");

    sv_addr.sin_family      = AF_INET;     // アドレスの種類 = インターネット
    sv_addr.sin_port        = htons(port); // ポート番号 = port
    sv_addr.sin_addr.s_addr = INADDR_ANY;  // 任意アドレスから受信可

    int opt = 1;
    setsockopt(rsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // ソケットオプションをセット

    // ソケットに設定を結び付ける
    if (bind(rsock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
        server_handle_error("bind()");
    }
    fprintf(stderr, "bind() is done successfully.\n");

    // 接続ソケットの準備
    if (listen(rsock, s_num_clients) != 0) {
        server_handle_error("listen()");
    }
    fprintf(stderr, "listen() is started.\n");

    int i, max_sock = 0;
    socklen_t len;
    char src[MAX_LEN_ADDR];

    // クライアントからサーバーの接続要求を処理する
    for (i = 0; i < s_num_clients; i++) {
        len  = sizeof(cl_addr);
        sock = accept(rsock, (struct sockaddr *)&cl_addr, &len);
        if (sock < 0) {
            server_handle_error("accept()");
        }
        if (max_sock < sock) {
            max_sock = sock;
        }
        if (read(sock, s_clients[i].name, MAX_LEN_NAME) == -1) {
            server_handle_error("read()");
        }
        s_clients[i].cid  = i;
        s_clients[i].sock = sock;
        s_clients[i].addr = cl_addr;
        memset(src, 0, sizeof(src));
        inet_ntop(AF_INET, (struct sockaddr *)&cl_addr.sin_addr, src, sizeof(src));
        fprintf(stderr, "Client %d is accepted (name=%s, address=%s, port=%d).\n", i, s_clients[i].name, src, ntohs(cl_addr.sin_port));
    }

    close(rsock);

    int j;
    for (i = 0; i < s_num_clients; i++) {
        server_send_data(i, &s_num_clients, sizeof(int));
        server_send_data(i, &i, sizeof(int));
        for (j = 0; j < s_num_clients; j++) {
            server_send_data(i, &s_clients[j], sizeof(CLIENT));
        }
    }

    s_num_socks = max_sock + 1;
    FD_ZERO(&s_mask);   // s_maskをゼロクリア
    FD_SET(0, &s_mask); // 0番目のFDに対応する値を1にセット

    // サーバーのセットアップ完了
    for (i = 0; i < s_num_clients; i++) {
        FD_SET(s_clients[i].sock, &s_mask);
    }
    fprintf(stderr, "Server setup is done.\n");

    // クライアントにスタートを知らせるコマンド送信
    // CONTAINER data;
    // data.command = START_COMMAND;
    // server_send_data(BROADCAST, &data, sizeof(CONTAINER));
}

/*******************************************************************
 * 関数名 : server_control_requests
 * 　　型 : int
 * 　説明 : データ受信制御を行う関数
 ******************************************************************/
int server_control_requests()
{
    fd_set read_flag = s_mask;
    memset(&s_data, 0, sizeof(CONTAINER));

    if (select(s_num_socks, (fd_set *)&read_flag, NULL, NULL, NULL) == -1) {
        server_handle_error("select()");
    }

    int i, result = 1;
    for (i = 0; i < s_num_clients; i++) {
        if (FD_ISSET(s_clients[i].sock, &read_flag)) {
            // データを受け取る
            server_receive_data(i, &s_data, sizeof(s_data));
            // データのコマンドによって条件分岐
            switch (s_data.command) {
            // メッセージが送られてきた場合
            case MESSAGE_COMMAND:
                // メッセージ内容をコンソールに表示
                fprintf(stderr, "client[%d] %s: message = %s\n", s_clients[i].cid, s_clients[i].name, s_data.message);
                // s_data をブロードキャスト
                server_send_data(BROADCAST, &s_data, sizeof(s_data));
                // 終了コードを result に格納
                result = 1;
                break;
            // 終了コマンドが送られてきた場合
            case QUIT_COMMAND:
                // 終了コマンドを送信したクライアントを表示
                fprintf(stderr, "client[%d] %s: quit\n", s_clients[i].cid, s_clients[i].name);
                // s_data をブロードキャスト
                server_send_data(BROADCAST, &s_data, sizeof(s_data));
                // 終了コードを result に格納
                result = 0;
                break;
            // それ以外のコマンドが送信された場合
            default:
                // 異常なエラーであることを表示する
                fprintf(stderr, "server_control_requests(): %c is not a valid command.\n", s_data.command);
                exit(1);
            }
        }
    }

    return result;
}

/*******************************************************************
 * 関数名 : server_send_data
 * 　　型 : void
 * 　引数 : cid（クライアントID）, s_data（データ構造体）
 *              size（データ構造体のサイズ）
 * 　説明 : データ受信制御を行う関数
 ******************************************************************/
void server_send_data(int cid, void *s_data, int size)
{
    if ((cid != BROADCAST) && (0 > cid || cid >= s_num_clients)) {
        // 異常終了
        fprintf(stderr, "server_send_data(): client id is illeagal.\n");
        exit(1);
    }
    if ((s_data == NULL) || (size <= 0)) {
        // 異常終了
        fprintf(stderr, "server_send_data(): data is illeagal.\n");
        exit(1);
    }

    if (cid == BROADCAST) {
        int i;
        for (i = 0; i < s_num_clients; i++) {
            if (write(s_clients[i].sock, s_data, size) < 0) {
                server_handle_error("write()");
            }
        }
    } else {
        if (write(s_clients[cid].sock, s_data, size) < 0) {
            server_handle_error("write()");
        }
    }
}

/*******************************************************************
 * 関数名 : server_receive_data
 * 　　型 : int
 * 　引数 : cid（クライアントID）, s_data（データ構造体）
 *              size（データ構造体のサイズ）
 * 　説明 : データを受信する関数
 ******************************************************************/
int server_receive_data(int cid, void *s_data, int size)
{
    if ((cid != BROADCAST) && (0 > cid || cid >= s_num_clients)) {
        // 異常終了
        fprintf(stderr, "server_receive_data(): client id is illeagal.\n");
        exit(1);
    }
    if ((s_data == NULL) || (size <= 0)) {
        // 異常終了
        fprintf(stderr, "server_receive_data(): data is illeagal.\n");
        exit(1);
    }

    return read(s_clients[cid].sock, s_data, size);
}

/*******************************************************************
 * 関数名 : server_handle_error
 * 　　型 : void
 * 　引数 : message（エラーメッセージ）
 * 　説明 : エラーを出力する関数
 ******************************************************************/
void server_handle_error(char *message)
{
    perror(message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}

/*******************************************************************
 * 関数名 : terminate_server
 * 　　型 : void
 * 　説明 : サーバーを終了する関数
 ******************************************************************/
void terminate_server(void)
{
    int i;
    for (i = 0; i < s_num_clients; i++) {
        close(s_clients[i].sock);
    }
    fprintf(stderr, "All connections are closed.\n");
}