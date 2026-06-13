/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_oled.h"
#include "Button.h"
#include "ESP.h"
#include "DoorOpenPro.h"
#include "RC522.h"
#include <string.h>
#include <stdio.h>
#include "FLASH.h"
#include "AdminOperation.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USER_FLASH_ADDR_PWD              0x08007800          // 扇区30起始地址（30KB位置）用于存储用户密码                  (密码)
#define USER_FLASH_ADDR_RFID             0x08007C00          // 扇区31起始地址（28KB位置）用于存储用户卡片ID                (用户ID)
#define USER_FLASH_ADDR_ADMIN_RFID       0x08008000          // 扇区32起始地址（24KB位置）用于存储管理员卡片ID              (管理员ID)
#define USER_FLASH_ADDR_RFID_New1        0x08008400          // 扇区33起始地址（20KB位置）用于存储新增加的卡片ID1           (新增用户ID1)
#define USER_FLASH_ADDR_RFID_New2        0x08008800          // 扇区34起始地址（16KB位置）用于存储新增加的卡片ID2           (新增用户ID2)
#define USER_FLASH_ADDR_ADMIN_RFID_New1  0x08008C00          // 扇区35起始地址（12KB位置）用于存储新增加的管理员卡片ID1      (新增管理员ID1)
#define USER_FLASH_ADDR_ADMIN_PWD        0x08009000          // 扇区36起始地址（8KB位置）用于存储管理员密码                 (管理员密码)

#define USER_FLASH_ADDR_INIT_FLAG        0x08009400          // 扇区37起始地址（4KB位置）用于存储初始化


#define USER_FLASH_ADDR_USER_CARD_COUNT  0x08007FF8          // 用户卡数量地址（扇区31末尾）
#define USER_FLASH_ADDR_ADMIN_CARD_COUNT 0x080083F8          // 管理员卡数量地址（扇区32末尾）

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  RC522_Init();
  Button_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  OLED_PrintString(2, 0, "ESP Init...");
	HAL_Delay(500);

	// 初始化ESP-01S（等待连接完成）
	if(ESP_Init()) {
		OLED_PrintString(2, 2, "ESP OK!");
	} else {
		OLED_PrintString(2, 2, "ESP Fail!");
	}
	HAL_Delay(1000);
	OLED_Clear();
  uint32_t user_ID = 0xBAD3411D;                //用户卡片ID
  uint32_t Administrator_ID = 0xC7F57B33;       //管理员卡片ID
  uint32_t user_pwd  = 123456;                  //密码
  uint32_t admin_pwd = 000000;                  //管理员密码
  uint32_t init_flag = Flash_Read(USER_FLASH_ADDR_INIT_FLAG);

	// 如果标志位不是预设值，说明是第一次运行，需要写入初始数据
	if(init_flag != 0x5A5A5A5A) {
		// 擦除所有使用的扇区
		Flash_Erase(USER_FLASH_ADDR_PWD);           // 扇区30 - 用户密码
		Flash_Erase(USER_FLASH_ADDR_RFID);          // 扇区31 - 用户卡
		Flash_Erase(USER_FLASH_ADDR_ADMIN_RFID);    // 扇区32 - 管理员卡
		Flash_Erase(USER_FLASH_ADDR_ADMIN_PWD);     // 扇区36 - 管理员密码
		Flash_Erase(USER_FLASH_ADDR_INIT_FLAG);     // 扇区37 - 初始化标志
		
		// 写入初始数据
		Flash_Write(USER_FLASH_ADDR_PWD, user_pwd);
		Flash_Write(USER_FLASH_ADDR_RFID, user_ID);
		Flash_Write(USER_FLASH_ADDR_ADMIN_RFID, Administrator_ID);
		Flash_Write(USER_FLASH_ADDR_ADMIN_PWD, admin_pwd);
		Flash_Write(USER_FLASH_ADDR_INIT_FLAG, 0x5A5A5A5A);
		
		// 初始化卡数量
		Flash_Write(USER_FLASH_ADDR_USER_CARD_COUNT, 1);
		Flash_Write(USER_FLASH_ADDR_ADMIN_CARD_COUNT, 1);
	}
  Regular_User();
  while (1)
  {
	// 处理ESP接收到的数据（放在后台运行）
    ESP_Process();
    
    // 每30秒发送心跳包
    static uint32_t last_heartbeat = 0;
    if(HAL_GetTick() - last_heartbeat > 30000) {
        ESP_SendHeartbeat();
        last_heartbeat = HAL_GetTick();
    }
    
    HAL_Delay(10);
    /* USER CODE END WHILE */
	}
    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
