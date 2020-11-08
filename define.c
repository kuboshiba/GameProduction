#include "header/define.h"

// エラーを色付きで出力する関数
void Error(char* message)
{
    printf("%sError: %s%s\n", COLOR_FG_LRED, message, COLOR_RESET);
}

// ログを色付きで出力する関数
void Log(char* message)
{
    printf("%sLog: %s%s\n", COLOR_FG_LMAGENTA, message, COLOR_RESET);
}

// map関数
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}