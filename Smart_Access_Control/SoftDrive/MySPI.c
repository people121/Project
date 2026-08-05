#include "stm32f1xx_hal.h"
#include "MySPI.h"
// PA4 - CS (片选)
// PA5 - SCK (时钟)
// PA6 - MISO (主机输入从机输出)
// PA7 - MOSI (主机输出从机输入)


/**
 * 函数名：MySPI_W_SS
 * 功能描述：设置片选引脚电平
 * 输入参数：BitValue --> 1:高电平, 0:低电平
 */
void MySPI_W_SS(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET));
}

/**
 * 函数名：MySPI_W_SCK
 * 功能描述：设置时钟引脚电平
 * 输入参数：BitValue --> 1:高电平, 0:低电平
 */
void MySPI_W_SCK(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET));
}

/**
 * 函数名：MySPI_W_MOSI
 * 功能描述：设置主机输出引脚电平
 * 输入参数：BitValue --> 1:高电平, 0:低电平
 */
void MySPI_W_MOSI(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET));
}

/**
 * 函数名：MySPI_R_MISO
 * 功能描述：读取主机输入引脚电平
 * 返回值：1-高电平, 0-低电平
 */
uint8_t MySPI_R_MISO(void)
{
    return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
}

/**
 * 函数名：MySPI_Init
 * 功能描述：初始化SPI引脚默认电平
 * 输入参数：无
 * 输出参数：无
 * 返回值：无
 */
void MySPI_Init(void)
{
    MySPI_W_SS(1);   // 片选默认高电平（不选中）
    MySPI_W_SCK(0);  // 时钟默认低电平
}

/**
 * 函数名：MySPI_Start
 * 功能描述：开始SPI通信（拉低片选）
 */
void MySPI_Start(void)
{
    MySPI_W_SS(0);
}

/**
 * 函数名：MySPI_Stop
 * 功能描述：停止SPI通信（拉高片选）
 */
void MySPI_Stop(void)
{
    MySPI_W_SS(1);
}

/**
 * 函数名：MySPI_SwapByte
 * 功能描述：通过软件SPI发送一个字节并接收一个字节
 * 输入参数：ByteSend --> 要发送的字节
 * 返回值：接收到的字节
 * 注意事项：模式0（CPOL=0, CPHA=0），时钟空闲低电平，上升沿采样
 */
uint8_t MySPI_SwapByte(uint8_t ByteSend)
{
    uint8_t i, ByteReceive = 0x00;
    
    for(i = 0; i < 8; i++)
    {
        // 1. 设置MOSI输出当前位（高位先发）
        MySPI_W_MOSI((ByteSend & 0x80) ? 1 : 0);
        ByteSend <<= 1;
        
        // 2. 时钟上升沿（从机采样）
        MySPI_W_SCK(1);
        
        // 3. 读取MISO（从机输出数据）
        ByteReceive <<= 1;
        if(MySPI_R_MISO())
        {
            ByteReceive |= 0x01;
        }
        
        // 4. 时钟下降沿（准备下一位）
        MySPI_W_SCK(0);
    }
    
    return ByteReceive;
}