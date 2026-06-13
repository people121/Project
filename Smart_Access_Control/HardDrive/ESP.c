/**
 * @file ESP.c
 * @brief ESP-01S模块控制程序，通过AT指令连接巴法云平台
 */

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "DoorOpenPro.h"
#include "AdminOperation.h"

/* 外部函数声明 */
extern UART_HandleTypeDef huart1;
extern uint16_t ESP_GetReceivedData(uint8_t* buf, uint16_t max_len);

/* 巴法云平台配置信息 */
char uid[]   = "997fb7889ff742939fe5860e41a201d1";
char topic[] = "led001";
char ssid[]  = "ACE";
char pswd[]  = "cr7204612";

/* 接收缓冲区（用于检查AT指令返回）*/
static uint8_t esp_rx_buf[256];
static uint16_t esp_rx_len = 0;
static uint16_t esp_rx_complete = 0;
/**
 * @brief 发送AT指令并等待指定字符串返回
 * @param cmd 指令
 * @param expected 期望返回的字符串（如 "OK"）
 * @param timeout_ms 超时时间(ms)
 * @return 1-找到期望字符串，0-超时/失败
 */
uint8_t ESP_SendCmdCheck(char *cmd, char *expected, uint16_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    
    /* 清空接收缓冲区标志 */
    esp_rx_complete = 0;
    esp_rx_len = 0;
    
    /* 发送指令 */
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 100);
    
    /* 等待返回 */
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (esp_rx_complete) {
            esp_rx_complete = 0;
            if (strstr((char*)esp_rx_buf, expected) != NULL) {
                return 1;
            }
        }
        HAL_Delay(10);
    }
    return 0;
}

/**
 * @brief 发送AT指令（不检查返回）
 */
void ESP_SendCmd(char *cmd) {
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 100);
    HAL_Delay(100);
}

void ESP_SendBemfaData(char *data) {
    HAL_UART_Transmit(&huart1, (uint8_t*)data, strlen(data), 100);
}

/**
 * @brief 初始化ESP-01S，带返回值检查
 * @return 1-成功，0-失败
 */
uint8_t ESP_Init(void) {
    char cmd[100];
    
    /* 1. 测试AT指令 */
    ESP_SendCmd("AT\r\n");
    HAL_Delay(500);
    
    /* 2. 复位模块 */
    ESP_SendCmd("AT+RST\r\n");
    HAL_Delay(3000);
    
    /* 3. 设置Station模式 */
    ESP_SendCmd("AT+CWMODE=1\r\n");
    HAL_Delay(500);
    
    /* 4. 连接WiFi */
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pswd);
    ESP_SendCmd(cmd);
    HAL_Delay(10000);
    
    /* 5. 查询IP，确认连接成功 */
    ESP_SendCmd("AT+CIFSR\r\n");
    HAL_Delay(1000);
    
    /* 6. 关闭已有连接 */
    ESP_SendCmd("AT+CIPCLOSE\r\n");
    HAL_Delay(500);
    
    /* 7. 设置单连接模式 */
    ESP_SendCmd("AT+CIPMUX=0\r\n");
    HAL_Delay(500);
    
    /* 8. 连接巴法云服务器 */
    ESP_SendCmd("AT+CIPSTART=\"TCP\",\"bemfa.com\",8344\r\n");
    HAL_Delay(3000);  // 等待连接结果
    
    /* 9. 设置透传模式 */
    ESP_SendCmd("AT+CIPMODE=1\r\n");
    HAL_Delay(500);
    
    /* 10. 进入透传模式 */
    ESP_SendCmd("AT+CIPSEND\r\n");
    HAL_Delay(500);
    
    /* 11. 发送订阅指令 */
    char sub_cmd[128];
    sprintf(sub_cmd, "cmd=1&uid=%s&topic=%s\r\n", uid, topic);
    ESP_SendBemfaData(sub_cmd);
    HAL_Delay(1000);
    
    return 1;
}

void ESP_SendHeartbeat(void) {
    char cmd[128];
    sprintf(cmd, "cmd=0&uid=%s&topic=%s\r\n", uid, topic);
    ESP_SendBemfaData(cmd);
}

void ESP_SendToBemfa(char *msg) {
    char cmd[256];
    sprintf(cmd, "cmd=2&uid=%s&topic=%s&msg=%s\r\n", uid, topic, msg);
    ESP_SendBemfaData(cmd);
}

void ESP_ParseData(char *data) {
    char *msg = strstr(data, "msg=");
    if (msg) {
        msg += 4;
        char *end = strchr(msg, '\r');
        if (!end) end = strchr(msg, '\n');
        if (!end) end = strchr(msg, '&');
        if (end) *end = '\0';
        
        /* 测试用：收到消息时闪烁LED或改变OLED显示 */
        if (strcmp(msg, "on") == 0) {
           Open_Door_Correct();
        } else if (strcmp(msg, "off") == 0) {
           Open_Door_Error();
        }
    }
}

void ESP_Process(void) {
    uint8_t buffer[256];
    uint16_t len;
    
    len = ESP_GetReceivedData(buffer, 255);
    if (len > 0) {
        buffer[len] = '\0';
        ESP_ParseData((char*)buffer);
    }
}