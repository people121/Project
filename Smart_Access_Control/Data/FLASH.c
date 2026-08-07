#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "FLASH.h"
#include "driver_oled.h"
#include <string.h>

/**
 * @brief 将字符串写入 Flash（最多 maxLen 字节，含结尾 '\0'）
 * @param Address  起始地址（必须按 4 字节对齐）
 * @param str      要写入的字符串
 * @param maxLen   最大长度（包含 '\0'），建议 <= 32
 */
void Flash_WriteString(uint32_t Address, const char *str, uint8_t maxLen)
{
    uint32_t data[8] = {0};          // 8 个 32 位字 = 32 字节
    uint8_t len = strlen(str);
    if (len > maxLen - 1) len = maxLen - 1;   // 保留 '\0' 位置
    memcpy(data, str, len);
    // 直接写入 8 个字（若 maxLen 小于 32，多余部分保持 0）
    for (int i = 0; i < 8; i++) {
        Flash_Write(Address + i * 4, data[i]);
    }
}

/**
 * @brief 从 Flash 读取字符串
 * @param Address  起始地址
 * @param buf      存放字符串的缓冲区
 * @param maxLen   缓冲区大小（包含 '\0'）
 */
void Flash_ReadString(uint32_t Address, char *buf, uint8_t maxLen)
{
    uint32_t data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = Flash_Read(Address + i * 4);
    }
    memcpy(buf, data, maxLen);
    buf[maxLen - 1] = '\0';
    
    // 检测是否全为 0xFF（未擦除/写入）
    int all_ff = 1;
    for (int i = 0; i < maxLen - 1; i++) {
        if (buf[i] != (char)0xFF) { all_ff = 0; break; }
    }
    if (all_ff) buf[0] = '\0';  // 设为空字符串
}
/*
 *  函数名：Flash_Erase
 *  功能描述：case 1: 擦除指定地址所在的页（如果地址在该页范围内）
 *           case 2: 擦除整个扇区（如果地址在该扇区范围内）
 *  输入参数：PageAddress --> 要擦除的页的起始地址或扇区的起始地址
 *  输出参数：无
 *  返回值：1-成功, 0-失败
 */
uint8_t Flash_Erase(uint32_t PageAddress)
{
    HAL_FLASH_Unlock();  // 解锁FLASH
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;  // 擦除类型：页擦除
    EraseInitStruct.PageAddress = PageAddress;           // 要擦除的页的起始地址
    EraseInitStruct.NbPages = 1;                        // 擦除1页
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();    // 锁定FLASH
        return 0; // 擦除失败
    }

    HAL_FLASH_Lock();    // 锁定FLASH
    return 1; // 擦除成功
}

/*
 *  函数名：HAL_StatusTypeDef
 *  功能描述: 写入数据到FLASH
 *  输入参数：Address --> 要写入的FLASH地址
 *           Data --> 要写入的数据（32位）
 *  输出参数：无
 *  返回值：HAL_OK-成功, HAL_ERROR-失败
 */
HAL_StatusTypeDef Flash_Write(uint32_t Address, uint32_t Data)
{
    HAL_FLASH_Unlock();  // 解锁FLASH
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, Data) != HAL_OK)
    {
        HAL_FLASH_Lock();    // 锁定FLASH
        return HAL_ERROR; // 写入失败
    }

    HAL_FLASH_Lock();    // 锁定FLASH
    return HAL_OK; // 写入成功
}
/*
 *  函数名：Flash_Read
 *  功能描述: 写入数据到FLASH
 *  输入参数：Address --> 要写入的FLASH地址
 *           Data --> 要写入的数据（32位）
 *  输出参数：无
 *  返回值：HAL_OK-成功, HAL_ERROR-失败
 */
uint32_t  Flash_Read(uint32_t Address)
{
    return *(__IO uint32_t *)Address; // 直接读取FLASH地址处的数据
}