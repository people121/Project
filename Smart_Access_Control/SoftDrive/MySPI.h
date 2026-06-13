#ifndef __MYSPI_H
#define __MYSPI_H

#include "stm32f1xx_hal.h"


/**
 * 函数名：MySPI_W_SS
 * 功能描述：设置片选引脚电平
 * 输入参数：BitValue --> 1:高电平, 0:低电平
 */
void MySPI_W_SS(uint8_t BitValue);

/**
 * 函数名：MySPI_W_SCK
 * 功能描述：设置时钟引脚电平
 * 输入参数：BitValue --> 1:高电平, 0:低电平
 */
void MySPI_W_SCK(uint8_t BitValue);

/**
 * 函数名：MySPI_W_MOSI
 * 功能描述：设置主机输出引脚电平
 * 输入参数：BitValue --> 1:高电平, 0:低电平
 */
void MySPI_W_MOSI(uint8_t BitValue);

/**
 * 函数名：MySPI_R_MISO
 * 功能描述：读取主机输入引脚电平
 * 返回值：1-高电平, 0-低电平
 */
uint8_t MySPI_R_MISO(void);

/**
 * 函数名：MySPI_Init
 * 功能描述：初始化SPI引脚默认电平
 * 输入参数：无
 * 输出参数：无
 * 返回值：无
 */
void MySPI_Init(void);

/**
 * 函数名：MySPI_Start
 * 功能描述：开始SPI通信（拉低片选）
 */
void MySPI_Start(void);
/**
 * 函数名：MySPI_Stop
 * 功能描述：停止SPI通信（拉高片选）
 */
void MySPI_Stop(void);

/**
 * 函数名：MySPI_SwapByte
 * 功能描述：通过软件SPI发送一个字节并接收一个字节
 * 输入参数：ByteSend --> 要发送的字节
 * 返回值：接收到的字节
 * 注意事项：模式0（CPOL=0, CPHA=0），时钟空闲低电平，上升沿采样
 */
uint8_t MySPI_SwapByte(uint8_t ByteSend);

#endif /* __MYSPI_H */