#ifndef __ESP_H
#define __ESP_H

#include "stm32f1xx_hal.h"

/*==========================================================================*/
/*                           ESP-01S 函数声明                               */
/*==========================================================================*/

/**
 * @brief 发送AT指令到ESP-01S模块
 * @param cmd 要发送的AT指令字符串（需包含\r\n）
 */
void ESP_SendCmd(char *cmd);

/**
 * @brief 发送巴法云协议数据（透传模式下使用）
 * @param data 要发送的数据字符串
 */
void ESP_SendBemfaData(char *data);

/**
 * @brief 初始化ESP-01S模块，连接WiFi和巴法云服务器
 * @return 1-成功，0-失败
 */
uint8_t ESP_Init(void);

/**
 * @brief 发送心跳包，保持在线状态（需要每隔30-40秒调用一次）
 */
void ESP_SendHeartbeat(void);

/**
 * @brief 发送消息到巴法云服务器
 * @param msg 要发送的消息（如 "on" 或 "off"）
 */
void ESP_SendToBemfa(char *msg);

/**
 * @brief 解析巴法云服务器返回的数据
 * @param data 从ESP-01S接收到的原始数据字符串
 */
void ESP_ParseData(char *data);

/**
 * @brief 处理ESP返回的数据（在主循环中周期性调用）
 */
void ESP_Process(void);

/**
 * @brief 获取ESP接收到的完整数据包（非阻塞）
 * @param buf 输出缓冲区
 * @param max_len 最大长度
 * @return 实际数据长度，0表示无数据
 */
uint16_t ESP_GetReceivedData(uint8_t* buf, uint16_t max_len);

#endif /* __ESP_H */