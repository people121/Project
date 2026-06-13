#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f1xx_hal.h"

// 定义按键值
#define KEY_NONE    0x00
#define KEY_1       '1'
#define KEY_2       '2'
#define KEY_3       '3'
#define KEY_4       '4'
#define KEY_5       '5'
#define KEY_6       '6'
#define KEY_7       '7'
#define KEY_8       '8'
#define KEY_9       '9'
#define KEY_0       '0'
#define KEY_UP      'U'//向上
#define KEY_DOWN    'D'//向下
#define KEY_Confirm 'C'//确定
#define KEY_ESC     'E'//退出
#define KEY_Rback   'R'//回退
#define KEY_HASH    '#'
// 初始化键盘
void Button_Init(void);

// 获取按下的按键（非阻塞，返回按键值，无按键返回0）
char Button_GetKey(void);

#endif

