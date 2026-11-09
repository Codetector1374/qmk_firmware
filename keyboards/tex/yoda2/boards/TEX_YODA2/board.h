#pragma once

#define BOARD_TEX_YODA2
#define BOARD_NAME "Tex Yoda 2"
#define HT32F1654
#define HT32_LSECLK 32768
#define HT32_HSECLK 8000000

#if !defined(_FROM_ASM_)
void boardInit(void);
#endif
