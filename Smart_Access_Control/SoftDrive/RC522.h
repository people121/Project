#ifndef __RC522_H
#define __RC522_H

#include <stdint.h>
#include "MySPI.h"      // 包含SPI相关函数
#include "RC522_Ins.h"  // 包含寄存器和命令定义

#define RC522_RST_PIN    GPIO_PIN_11   // 复位引脚（PA11）
#define RC522_RST_PORT   GPIOA         // 复位引脚端口

// SPI引脚操作（直接调用MySPI函数）
#define RC522_SDA_L      MySPI_W_SS(0)    // 片选拉低（选中RC522）
#define RC522_SDA_H      MySPI_W_SS(1)    // 片选拉高（释放RC522）


/**
 * 函数名：RC522_Init
 * 功能描述：初始化RC522模块，配置GPIO和寄存器
 * 输入参数：无
 * 返回值：无
 */
void RC522_Init(void);

/**
 * 函数名：Read_RC522
 * 功能描述：从RC522指定寄存器读取一个字节
 * 输入参数：Address - 寄存器地址
 * 返回值：读取到的寄存器值
 */
unsigned char Read_RC522(unsigned char Address);

/**
 * 函数名：Write_RC522
 * 功能描述：向RC522指定寄存器写入一个字节
 * 输入参数：Address - 寄存器地址，value - 要写入的值
 * 返回值：无
 */
void Write_RC522(unsigned char Address, unsigned char value);

/**
 * 函数名：RC522_Reset
 * 功能描述：复位RC522芯片
 * 输入参数：无
 * 返回值：MI_OK-成功
 */
char RC522_Reset(void);

/**
 * 函数名：SetBitMask
 * 功能描述：设置寄存器中的指定位（置1）
 * 输入参数：reg - 寄存器地址，mask - 位掩码
 * 返回值：无
 */
void SetBitMask(unsigned char reg, unsigned char mask);

/**
 * 函数名：ClearBitMask
 * 功能描述：清除寄存器中的指定位（置0）
 * 输入参数：reg - 寄存器地址，mask - 位掩码
 * 返回值：无
 */
void ClearBitMask(unsigned char reg, unsigned char mask);

/**
 * 函数名：RC522_AntennaOn
 * 功能描述：开启RC522天线
 * 输入参数：无
 * 返回值：无
 */
void RC522_AntennaOn(void);

/**
 * 函数名：RC522_AntennaOff
 * 功能描述：关闭RC522天线
 * 输入参数：无
 * 返回值：无
 */
void RC522_AntennaOff(void);

/**
 * 函数名：RC522_ReadCardID
 * 功能描述：读取卡片ID（寻卡+防冲突）
 * 输入参数：uid - 用于存储4字节卡片UID的缓冲区
 * 返回值：MI_OK-成功，MI_ERR-失败
 */
char RC522_ReadCardID(unsigned char *uid);

/**
 * 函数名：RC522_Halt
 * 功能描述：让卡片进入休眠状态，以便下次重新寻卡
 * 输入参数：无
 * 返回值：MI_OK-成功
 */
char RC522_Halt(void);

#endif /* __RC522_H */