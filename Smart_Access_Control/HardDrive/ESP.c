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
#include "FLASH.h"   // 用于读取Flash

#define USER_FLASH_ADDR_RFID             0x08007C00         // 扇区31起始地址（28KB位置）用于存储用户卡片ID                (用户ID)
#define USER_FLASH_ADDR_ADMIN_RFID       0x08008000         // 扇区32起始地址（24KB位置）用于存储管理员卡片ID              (管理员ID)
#define USER_FLASH_ADDR_RFID_New1        0x08008400         // 扇区33起始地址（20KB位置）用于存储新增加的卡片ID1           (新增用户ID1)
#define USER_FLASH_ADDR_RFID_New2        0x08008800         // 扇区34起始地址（16KB位置）用于存储新增加的卡片ID2           (新增用户ID2)
#define USER_FLASH_ADDR_ADMIN_RFID_New1  0x08008C00         // 扇区35起始地址（12KB位置）用于存储新增加的管理员卡片ID1      (新增管理员ID1)
#define USER_FLASH_ADDR_WIFI_ID          0x08009800   		// WiFi 名称
#define USER_FLASH_ADDR_WIFI_PWD         0x08009C00   		// WiFi 密码
/* 外部函数声明 */
extern UART_HandleTypeDef huart1;
extern uint16_t ESP_GetReceivedData(uint8_t* buf, uint16_t max_len);
void ESP_ReportCardList(void);
void ESP_ReconnectWiFi(char *ssid, char *password);
/* 巴法云平台配置信息 */
char uid[]   = "997fb7889ff742939fe5860e41a201d1";
char topic[] = "led001";
// char ssid[]  = "A";
// char pswd[]  = "cr7204612";
//char ssid[] == Flash_Read(USER_FLASH_ADDR_WIFI)

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
    char ssid[32] = {0};
    char pwd[32] = {0};
    char cmd[100];

    // 从 Flash 读取 WiFi 信息（如果未初始化，Flash_ReadString 会返回空字符串）
    Flash_ReadString(USER_FLASH_ADDR_WIFI_ID, ssid, 32);
	Flash_ReadString(USER_FLASH_ADDR_WIFI_PWD, pwd, 32);
    // 如果读出来的 SSID 为空，使用默认值（与 main.c 中的默认值一致）
    if (strlen(ssid) == 0) {
        strcpy(ssid, "A");
        strcpy(pwd, "cr7204612");
    }

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
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    ESP_SendCmd(cmd);
    HAL_Delay(10000);   // 等待连接（可根据实际调整）
    
    /* 5. 查询IP（可选） */
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
    HAL_Delay(3000);
    
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

/**
 * @brief 读取所有卡片ID，打包后上报到 cardlist topic
 */
void ESP_ReportCardList(void) {
    char card_str[256] = "cards:";
    char temp[16];
    uint32_t ids[] = {
        Flash_Read(USER_FLASH_ADDR_RFID),
        Flash_Read(USER_FLASH_ADDR_ADMIN_RFID),
        Flash_Read(USER_FLASH_ADDR_RFID_New1),
        Flash_Read(USER_FLASH_ADDR_RFID_New2),
        Flash_Read(USER_FLASH_ADDR_ADMIN_RFID_New1)
    };
    int first = 1;
    for (int i = 0; i < 5; i++) {
        if (ids[i] != 0xFFFFFFFF) {
            if (!first) strcat(card_str, ",");
            sprintf(temp, "%08lX", (unsigned long)ids[i]);
            strcat(card_str, temp);
            first = 0;
        }
    }
    if (first) strcpy(card_str, "none");

    char cmd[300];
    sprintf(cmd, "cmd=2&uid=%s&topic=led001&msg=%s\r\n", uid, card_str);
    ESP_SendBemfaData(cmd);
}
void ESP_ParseData(char *data) {
    char *msg = strstr(data, "msg=");
    if (!msg) return;
    msg += 4;
    char *end = strchr(msg, '\r');
    if (!end) end = strchr(msg, '\n');
    if (!end) end = strchr(msg, '&');
    if (end) *end = '\0';

    // 开门 / 关门
    if (strcmp(msg, "on") == 0) {
        Open_Door_Correct();
        return;
    } else if (strcmp(msg, "off") == 0) {
        Open_Door_Error();
        return;
    }

    // 查看卡片列表
	if (strstr(msg, "list_cards") != NULL) {
		OLED_PrintString(0, 6, "Got list     ");
		Show_Card_List();   // 改为本地 OLED 显示
		return;
	}

    // 管理卡片：add_user:XXXXXXXX, add_admin:XXXXXXXX, del_user:XXXXXXXX, del_admin:XXXXXXXX
    uint32_t card_id = 0;
    char action[16] = {0};
    int parsed = 0;
    if (strncmp(msg, "add_user:", 9) == 0) {
        strcpy(action, "add_user");
        sscanf(msg + 9, "%8lX", &card_id);
        parsed = 1;
    } else if (strncmp(msg, "add_admin:", 10) == 0) {
        strcpy(action, "add_admin");
        sscanf(msg + 10, "%8lX", &card_id);
        parsed = 1;
    } else if (strncmp(msg, "del_user:", 9) == 0) {
        strcpy(action, "del_user");
        sscanf(msg + 9, "%8lX", &card_id);
        parsed = 1;
    } else if (strncmp(msg, "del_admin:", 10) == 0) {
        strcpy(action, "del_admin");
        sscanf(msg + 10, "%8lX", &card_id);
        parsed = 1;
    }
    if (parsed && card_id != 0) {
        if (strcmp(action, "add_user") == 0) Add_User_Card_Direct(card_id);
        else if (strcmp(action, "add_admin") == 0) Add_Admin_Card_Direct(card_id);
        else if (strcmp(action, "del_user") == 0) Delete_User_Card_Direct(card_id);
        else if (strcmp(action, "del_admin") == 0) Delete_Admin_Card_Direct(card_id);
        // 操作完成后上报最新卡片列表
        ESP_ReportCardList();
        return;
    }

    // 修改密码：change_user_pwd:123456 或 change_admin_pwd:111111
    uint32_t new_pwd = 0;
    if (strncmp(msg, "change_user_pwd:", 16) == 0) {
        sscanf(msg + 16, "%6lu", &new_pwd);
        if (new_pwd >= 100000 && new_pwd <= 999999) {
            Change_Password_Direct(0, new_pwd);
        }
    } else if (strncmp(msg, "change_admin_pwd:", 17) == 0) {
        sscanf(msg + 17, "%6lu", &new_pwd);
        if (new_pwd >= 100000 && new_pwd <= 999999) {
            Change_Password_Direct(1, new_pwd);
        }
    }
	    // ---- WiFi 配网 ----
    if (strncmp(msg, "wifi:", 5) == 0) {
        char *comma = strchr(msg + 5, ',');
        if (comma) {
            char new_ssid[32] = {0};
            char new_pwd[32] = {0};
            int ssid_len = comma - (msg + 5);
            if (ssid_len > 31) ssid_len = 31;
            strncpy(new_ssid, msg + 5, ssid_len);
            strncpy(new_pwd, comma + 1, 31);
            // 调用重连函数
            ESP_ReconnectWiFi(new_ssid, new_pwd);
        }
        return;
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
/**
 * @brief 更新 WiFi 信息并重新连接（参照修改密码的方式）
 * @param ssid     新 SSID
 * @param password 新密码
 */
void ESP_ReconnectWiFi(char *ssid, char *password)
{
	Flash_Erase(USER_FLASH_ADDR_WIFI_ID);
	Flash_Erase(USER_FLASH_ADDR_WIFI_PWD);
	Flash_WriteString(USER_FLASH_ADDR_WIFI_ID, ssid, 32);
	Flash_WriteString(USER_FLASH_ADDR_WIFI_PWD, password, 32);

    // 3. OLED 提示
    OLED_PrintString(0, 6, "WiFi Updating");

    // 4. 退出透传（发送 +++ 三次，不加换行）
    for (int i = 0; i < 3; i++) {
        HAL_UART_Transmit(&huart1, (uint8_t*)"+++", 3, 100);
        HAL_Delay(200);
    }
    HAL_Delay(1000);   // 等待切换到命令模式

    // 5. 复位 ESP 模块（如果在命令模式，此指令生效）
    ESP_SendCmd("AT+RST\r\n");
    HAL_Delay(3000);

    // 6. 重新初始化（会从 Flash 读取新信息）
    ESP_Init();

    // 7. OLED 提示完成
    OLED_PrintString(0, 6, "WiFi Updated ");
}