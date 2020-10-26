#include "header/define.h"

// エラーを色付きで出力する関数
void Error(char* message)
{
    printf("%sError: %s%s\n", COLOR_FG_LRED, message, COLOR_RESET);
}

// ログを色付きで出力する関数
void Log(char* message)
{
    printf("%sLog: %s%s\n", COLOR_FG_LBLUE, message, COLOR_RESET);
}