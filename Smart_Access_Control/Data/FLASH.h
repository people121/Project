#ifndef __FLASH_H
#define __FLASH_H

/*
 *  函数名：Flash_Erase
 *  功能描述：case 1: 擦除指定地址所在的页（如果地址在该页范围内）
 *           case 2: 擦除整个扇区（如果地址在该扇区范围内）
 *  输入参数：PageAddress --> 要擦除的页的起始地址或扇区的起始地址
 *  输出参数：无
 *  返回值：1-成功, 0-失败
 */
uint8_t Flash_Erase(uint32_t PageAddress);

/*
 *  函数名：HAL_StatusTypeDef
 *  功能描述: 写入数据到FLASH
 *  输入参数：Address --> 要写入的FLASH地址
 *           Data --> 要写入的数据（32位）
 *  输出参数：无
 *  返回值：HAL_OK-成功, HAL_ERROR-失败
 */
HAL_StatusTypeDef Flash_Write(uint32_t Address, uint32_t Data);
/*
 *  函数名：Flash_Read
 *  功能描述: 从FLASH读取数据
 *  输入参数：Address --> 要读取的FLASH地址
 *  输出参数：无
 *  返回值：HAL_OK-成功, HAL_ERROR-失败
 */
uint32_t  Flash_Read(uint32_t Address);



#endif  /* USER CODE BEGIN 1 */
