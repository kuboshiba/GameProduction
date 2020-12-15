#include "header/define.h"

/*******************************************************************
 * 関数名 : Error
 * 　　型 : void
 * 　引数 : char* message (出力したい文字列)
 * 　説明 : エラー内容を色付きでターミナルに標準出力する
 ******************************************************************/
void Error(char* message)
{
    printf("%sError: %s%s\n", COLOR_FG_LRED, message, COLOR_RESET);
}

/*******************************************************************
 * 関数名 : Log
 * 　　型 : void
 * 　引数 : char* message (出力したい文字列)
 * 　説明 : ログを色付きでターミナルに標準出力する
 ******************************************************************/
void Log(char* message)
{
    printf("%sLog: %s%s\n", COLOR_FG_LMAGENTA, message, COLOR_RESET);
}

/*******************************************************************
 * 関数名 : SystemLog
 * 　　型 : void
 * 　引数 : char* message (出力したい文字列)
 * 　説明 : システムログを色付きでターミナルに標準出力する
 ******************************************************************/
void SystemLog(char* message)
{
    printf("%sSystemLog: %s%s\n", COLOR_FG_LBLUE, message, COLOR_RESET);
}

/*******************************************************************
 * 関数名 : map
 * 　　型 : int
 *   引数 : int 入力値，最小値（入力），最大値（入力），
 *              最小値（出力），最大値（出力）
 * 　戻値 : int (線形変換した値)
 * 　説明 : 数値を範囲を指定し線形変換する
 ******************************************************************/
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}