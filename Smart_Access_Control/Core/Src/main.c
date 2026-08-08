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
#include "cmsis_os.h"
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
#include "stm32f1xx_hal_wwdg.h"
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
#define USER_FLASH_ADDR_INIT_FLAG        0x08009400          // 扇区37起始地址（4KB位置）用于存储初始化标志
#define USER_FLASH_ADDR_WIFI_ID          0x08009800          // 扇区38起始地址（0KB位置）用于存储WiFi信息                 （名称）
#define USER_FLASH_ADDR_WIFI_PWD         0x08009C00          // 扇区39起始地址（0KB位置）用于存储WiFi信息                 （密码）

#define USER_FLASH_ADDR_USER_CARD_COUNT  0x08007FF8          // 用户卡数量地址（扇区31末尾）
#define USER_FLASH_ADDR_ADMIN_CARD_COUNT 0x080083F8          // 管理员卡数量地址（扇区32末尾）

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TIM_HandleTypeDef htim2;
volatile uint8_t wdg_feed_flag = 0;   // 喂狗标志
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);      // 系统时钟配置函数
void MX_FREERTOS_Init(void);        // FreeRTOS初始化函数
/* USER CODE BEGIN PFP */
void MX_TIM2_Init(void);            // 定时器2初始化函数（用于看门狗喂养）
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
  OLED_PrintString(2, 0, "ESP Init...");
	HAL_Delay(500);

	// 初始化ESP-01S（等待连接完成）
	if(ESP_Init()) {
		OLED_PrintString(2, 2, "ESP OK!");
	} else {
		OLED_PrintString(2, 2, "ESP Fail!");
	}
	HAL_Delay(1000);
  

  /* 启动PWM（舵机） */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);                           // 启动PWM输出
  TIM1->BDTR |= TIM_BDTR_MOE;                                         // 主输出使能
  
  /* Flash初始化（第一次运行写入默认数据） */
  uint32_t user_ID = 0xBAD3411D;                                      // 用户卡片ID
  uint32_t Administrator_ID = 0xC7F57B33;                             // 管理员卡片ID
  uint32_t user_pwd = 123456;                                         // 用户密码
  uint32_t admin_pwd =111111;                                         // 管理员密码
  char ssid[]  = "A";
  char pswd[]  = "cr7204612";

  uint32_t init_flag = Flash_Read(USER_FLASH_ADDR_INIT_FLAG);         // 读取初始化标志
  
  if(init_flag != 0x5A5A5A5A) {                                       // 如果未初始化，则写入默认数据
      Flash_Erase(USER_FLASH_ADDR_PWD);                               // 擦除用户密码存储区域
      Flash_Erase(USER_FLASH_ADDR_RFID);                              // 擦除用户卡片ID存储区域
      Flash_Erase(USER_FLASH_ADDR_ADMIN_RFID);                        // 擦除管理员卡片ID存储区域
      Flash_Erase(USER_FLASH_ADDR_ADMIN_PWD);                         // 擦除管理员密码存储区域
      Flash_Erase(USER_FLASH_ADDR_INIT_FLAG);                         // 擦除初始化标志存储区域
      Flash_Erase(USER_FLASH_ADDR_USER_CARD_COUNT);                   // 擦除用户卡数量存储区域
      Flash_Erase(USER_FLASH_ADDR_ADMIN_CARD_COUNT);                  // 擦除管理员卡数量
      Flash_Erase(USER_FLASH_ADDR_WIFI_ID);                           // 擦除WiFi名称存储区域
      Flash_Erase(USER_FLASH_ADDR_WIFI_PWD);                          // 擦除WiFi密码存储区域

      Flash_Write(USER_FLASH_ADDR_PWD, user_pwd);                     // 写入用户密码
      Flash_Write(USER_FLASH_ADDR_RFID, user_ID);                     // 写入用户卡片ID
      Flash_Write(USER_FLASH_ADDR_ADMIN_RFID, Administrator_ID);      // 写入管理员卡片ID
      Flash_Write(USER_FLASH_ADDR_ADMIN_PWD, admin_pwd);              // 写入管理员密码
      Flash_Write(USER_FLASH_ADDR_INIT_FLAG, 0x5A5A5A5A);             // 写入初始化标志
	  Flash_WriteString(USER_FLASH_ADDR_WIFI_ID, ssid, 32); 
	  Flash_WriteString(USER_FLASH_ADDR_WIFI_PWD, pswd, 32);          // 密码（偏移 32 字节）
      
      Flash_Write(USER_FLASH_ADDR_USER_CARD_COUNT, 1);                // 写入用户卡数量为1      
      Flash_Write(USER_FLASH_ADDR_ADMIN_CARD_COUNT, 1);               // 写入管理员卡数量为1
  }
  /* USER CODE END 2 */
  wdg_feed_flag = 1;          // 初始化喂狗标志
  MX_TIM2_Init();             // 初始化定时器2（用于看门狗喂养）
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */
	
    /* USER CODE BEGIN 3 */
  }
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
void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7200 - 1;      // 72MHz/7200 = 10kHz
    htim2.Init.Period = 10000 - 1;        // 10kHz / 10000 = 1Hz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }
    // 设置中断优先级并启用中断
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);   // 设置优先级（数值越小优先级越高）
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    HAL_TIM_Base_Start_IT(&htim2);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        if (wdg_feed_flag == 0) {
            // 超时未喂狗，软件复位
            NVIC_SystemReset();
        } else {
            wdg_feed_flag = 0;   // 清零，等待下次喂狗
        }
    }
}
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
