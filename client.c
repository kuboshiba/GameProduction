#include "header/define.h"

int client_main()
{
    u_short port = DEFAULT_PORT;
    char server_name[MAX_LEN_NAME];

    sprintf(server_name, "localhost");

    setup_client(server_name, port); // クライアントのセットアップを行う

    // メインループ
    int cond = 1;
    while (cond) {
        cond = client_control_requests();
    }

    terminate_client(); // クライアントの終了処理を行う

    return 0;
}

// クライアント側のセットアップ
void setup_client(char *server_name, u_short port)
{
    struct hostent *server;
    struct sockaddr_in sv_addr;

    // サーバーに接続を試みる
    fprintf(stderr, "Trying to connect server %s (port = %d).\n", server_name, port);
    if ((server = gethostbyname(server_name)) == NULL) {
        // 異常終了
        client_handle_error("gethostbyname()");
    }

    // ストリーム接続を試みる
    c_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (c_sock < 0) {
        // 異常終了
        client_handle_error("socket()");
    }

    sv_addr.sin_family = AF_INET;     // アドレスの種類 = インターネット
    sv_addr.sin_port   = htons(port); // ポート番号 = port
    // sv_addr.sin_addr.s_addr = *(u_int *)server->h_addr_list[0]; // 任意アドレスから受信可
    inet_aton("192.168.64.88", &sv_addr.sin_addr);

    // サーバーへの通信リクエスト
    if (connect(c_sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
        // 異常終了
        client_handle_error("connect()");
    }

    // 名前の入力
    fprintf(stderr, "Input your name: ");
    char user_name[MAX_LEN_NAME];

    // 名前の文字数チェック
    if (fgets(user_name, sizeof(user_name), stdin) == NULL) {
        // 異常終了
        client_handle_error("fgets()");
    }

    gGame.mode = MD_MULTI_CLIENT_2;

    user_name[strlen(user_name) - 1] = '\0';
    client_send_data(user_name, MAX_LEN_NAME); // 名前を送信

    // 他のクライアントを待つ
    fprintf(stderr, "Waiting for other clients...\n");
    client_receive_data(&c_num_clients, sizeof(int));
    fprintf(stderr, "Number of clients = %d.\n", c_num_clients);
    client_receive_data(&c_myid, sizeof(int));
    fprintf(stderr, "Your ID = %d.\n", c_myid);
    int i;
    for (i = 0; i < c_num_clients; i++) {
        client_receive_data(&c_clients[i], sizeof(CLIENT));
    }

    c_num_sock = c_sock + 1;
    FD_ZERO(&c_mask);        // c_maskをゼロクリア
    FD_SET(0, &c_mask);      // 0番目のFDに対応する値を1にセット
    FD_SET(c_sock, &c_mask); // c_sockのFDに対応する値を1にセット
    fprintf(stderr, "Input command (M=message, Q=quit): \n");
}

// データ受信制御を行う関数
int client_control_requests()
{
    fd_set read_flag = c_mask;

    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 30;

    if (select(c_num_sock, (fd_set *)&read_flag, NULL, NULL, &timeout) == -1) {
        client_handle_error("select()");
    }

    // FDが１がどうか確認
    int result = 1;
    if (FD_ISSET(0, &read_flag)) {
        result = in_command();
    } else if (FD_ISSET(c_sock, &read_flag)) {
        result = exe_command();
    }

    return result;
}

// コマンドの入力を行う関数
int in_command()
{
    CONTAINER data;
    char com;
    memset(&data, 0, sizeof(CONTAINER));
    com = getchar();
    while (getchar() != '\n')
        ;

    // コマンドによって条件分岐
    switch (com) {
    // メッセージコマンドの場合
    case MESSAGE_COMMAND:
        // メッセージを入力して送信する
        fprintf(stderr, "Input message: ");
        if (fgets(data.message, MAX_LEN_BUFFER, stdin) == NULL) {
            client_handle_error("fgets()");
        }
        data.command                           = MESSAGE_COMMAND;
        data.message[strlen(data.message) - 1] = '\0';
        data.cid                               = c_myid;
        client_send_data(&data, sizeof(CONTAINER));
        break;
    // 終了コマンドの場合
    case QUIT_COMMAND:
        data.command = QUIT_COMMAND;
        data.cid     = c_myid;
        client_send_data(&data, sizeof(CONTAINER));
        break;
    }

    return 1;
}

// コマンドを実行する関数
int exe_command()
{
    int result = 1;
    memset(&c_data, 0, sizeof(CONTAINER));
    client_receive_data(&c_data, sizeof(c_data));

    // data.command によって条件分岐
    switch (c_data.command) {
    // メッセージコマンドの場合
    case MESSAGE_COMMAND:
        fprintf(stderr, "client[%d] %s: %s\n", c_data.cid, c_clients[c_data.cid].name, c_data.message);
        result = 1;
        break;
    // 終了コマンドの場合
    case QUIT_COMMAND:
        fprintf(stderr, "client[%d] %s sent quit command.\n", c_data.cid, c_clients[c_data.cid].name);
        result = 0;
        break;
    case START_COMMAND:
        gGame.mode = MD_MULTI_CLIENT_3;
        break;
    case DATA_COMMAND:
        for (int i = 0; i < 10; i++) {
            printf("%d ", c_data.target[i].type);
        }
        printf("\n");
        break;
    }

    return result;
}

// データの送信を行う関数
void client_send_data(void *data, int size)
{
    if ((data == NULL) || (size <= 0)) {
        fprintf(stderr, "client_send_data(): data is illeagal.\n");
        exit(1);
    }

    if (write(c_sock, data, size) == -1) {
        client_handle_error("write()");
    }
}

// データの受信を行う関数
int client_receive_data(void *data, int size)
{
    if ((data == NULL) || (size <= 0)) {
        fprintf(stderr, "client_receive_data(): data is illeagal.\n");
        exit(1);
    }

    return (read(c_sock, data, size));
}

// エラー内容の出力処理
void client_handle_error(char *message)
{
    perror(message);
    fprintf(stderr, "%d\n", errno);
    exit(1);
}

// クライアントの終了処理
void terminate_client()
{
    fprintf(stderr, "Connection is closed.\n");
    close(c_sock);
    exit(0);
}
