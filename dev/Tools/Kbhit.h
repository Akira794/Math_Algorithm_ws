#ifndef KBHIT_H
#define KBHIT_H

#ifdef WINDOWS
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
void KB_open(void);
void KB_close(void);
bool KB_hit(void);
char KB_getch(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* KBHIT_H */
