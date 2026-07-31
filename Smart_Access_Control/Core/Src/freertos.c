/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"       // FreeRTOS任务管理头文件
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "driver_oled.h"
#include "ESP.h"
#include "DoorOpenPro.h"
#include "gpio.h"
#include "RC522.h"
#include "AdminOperation.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern unsigned char user_uid[4];                   // 存储读到的卡片ID
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;     // 默认任务句柄
osThreadId userTaskHandle;        // 用户任务句柄
osThreadId rc522TaskHandle;       // RC522任务句柄
osThreadId regularTaskHandle;     // 用户界面任务句柄
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);   // 默认任务函数
void StartUserTask(void const * argument);      // 用户任务函数
void StartRC522Task(void const * argument);     // RC522任务函数
void StartRegularTask(void const * argument);   // 用户界面任务函数

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  
  /* 默认任务（空闲/后台） */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL); 

  /* 界面刷新任务（低优先级，不卡顿即可） */
  osThreadDef(regularTask, StartRegularTask, osPriorityLow, 0, 256);
  regularTaskHandle = osThreadCreate(osThread(regularTask), NULL);
	if (regularTaskHandle == NULL){
		    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			osDelay(500);
	} 

  /* RC522刷卡任务（中优先级，有延时） */
  osThreadDef(rc522Task, StartRC522Task, osPriorityNormal, 0, 256);
  rc522TaskHandle = osThreadCreate(osThread(rc522Task), NULL);
	if(rc522TaskHandle == NULL){
		while (1) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
			HAL_Delay(200);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
			HAL_Delay(200);
		}
	}

  /* ESP通信任务（高优先级，处理网络数据） */
  osThreadDef(userTask, StartUserTask, osPriorityAboveNormal, 0, 512);
  userTaskHandle = osThreadCreate(osThread(userTask), NULL);
	if(userTaskHandle == NULL){
		 while (1) {
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
			HAL_Delay(200);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
			HAL_Delay(200);
		}
	}  

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}
/*
 *  函数名：StartUserTask
 *  功能描述： 处理ESP通信任务，包括接收和发送数据，以及定时发送心跳包。
 *  参数 ：argument - 任务参数（未使用）
 *  返回值：无
 */
void StartUserTask(void const * argument)
{
  osDelay(500); 
  OLED_PrintString(0, 2, "ESP Task"); // 显示ESP初始化信息
  for(;;)
  {
    ESP_Process();                    // 处理ESP通信任务

    /* 每15秒发送心跳包 */
    static uint32_t last_heartbeat_time = 0;            // 上次发送心跳包的时间
    if (HAL_GetTick() - last_heartbeat_time >= 15000) { // 检查是否已过15秒
        last_heartbeat_time = HAL_GetTick();            // 更新上次发送心跳包的时间
        ESP_SendHeartbeat();                            // 发送心跳包
    }
    osDelay(100);                                       // 延时100ms，避免任务占用过多CPU资源
  }
}
/*
 *  函数名：StartRC522Task
 *  功能描述： 处理RC522刷卡任务，包括检测刷卡、读取卡片信息，并根据卡片信息进行相应的处理。
 *  参数 ：argument - 任务参数（未使用）
 *  返回值：无
 */
void StartRC522Task(void const * argument)
{
    for(;;){
        if(RC522_ReadCardID(user_uid) == MI_OK){
            Uid_Read();                    // 处理刷卡
            RC522_Halt();                  // 让卡片休眠，避免重复读取
        }
        osDelay(100);                      // 每100ms扫描一次，释放CPU
    }
}

void StartRegularTask(void const * argument)
{
  OLED_Clear();
  OLED_PrintString(0, 0, "Smart Access");
  OLED_PrintString(0, 2, "Swipe Card...");
  osDelay(500);
  
  for(;;)
  {
    /* 进入用户主界面 */
    Regular_User();
    
    osDelay(50);
  }
}
/* Private application code ------------------------------------------------__*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

