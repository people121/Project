#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "FLASH.h"
#include "driver_oled.h"

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