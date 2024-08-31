#include "Kbhit.h"
#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef WINDOWS
#else
RBSTATIC struct termios old_set;
RBSTATIC struct termios new_set;
RBSTATIC int32_t read_char = -1;
#endif

void KB_open(void)
{
    #ifdef WINDOWS
    #else
    tcgetattr(0,&old_set);/* 現在の画面環境の取得 */
    new_set = old_set; /* 現在の画面環境の保存 */
    new_set.c_lflag &= ~ICANON; /* 現在の画面環境を非カノニカル・モード 及び エコーオフに設定 */
    new_set.c_lflag &= ~ECHO;
    new_set.c_lflag &= ~ISIG;
    new_set.c_cc[VMIN] = 0; /* 設定用のパラメータ */
    new_set.c_cc[VTIME] = 0;
    tcsetattr(0,TCSANOW,&old_set); /* 現在の画面環境を更新 */
    #endif

}

void KB_close(void) /*保存しておいた環境で現在の画面環境を更新(もとの環境に戻す)*/
{
    #ifdef WINDOWS
    #else
    tcsetattr(0,TCSANOW, &old_set);
    #endif
}

bool KB_hit(void)/* キーイベント取得関数 */
{
    #ifdef WINDOWS
    return _kbhit();
    #else
    char ch;
    int32_t nread;

    if(read_char !=-1)
    {
        return true;
    }

    new_set.c_cc[VMIN]=0;
    tcsetattr(0,TCSANOW,&new_set);
    nread=read(0,&ch,1);
    new_set.c_cc[VMIN]=1;
    tcsetattr(0,TCSANOW,&new_set);

    if(nread == 1)
    {
        read_char = ch;
        return true;
    }
    return false;
    #endif
}

char KB_getch(void)/* char型の取得関数 */
{
    #ifdef WINDOWS
    return _getch();
    #else
    char ch;

    if(read_char != -1)
    {
        ch = read_char;
        read_char = -1;
        return (ch);
    }

    read(0,&ch,1);
    return(ch);
    #endif
}
