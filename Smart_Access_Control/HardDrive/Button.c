#include "stm32f1xx_hal.h"
#include "button.h"
#include "gpio.h"

/*定义行线（输出）引脚 */
#define ROW1_PIN    GPIO_PIN_0
#define ROW2_PIN    GPIO_PIN_1
#define ROW3_PIN    GPIO_PIN_2
#define ROW4_PIN    GPIO_PIN_3
#define ROW_PORT    GPIOA

/* 定义列线（输入）引脚 */
#define COL1_PIN    GPIO_PIN_0
#define COL2_PIN    GPIO_PIN_1
#define COL3_PIN    GPIO_PIN_3
#define COL4_PIN    GPIO_PIN_4
#define COL_PORT    GPIOB

/* 按键映射表（4x4矩阵）*/
static const char key_map[4][4] = {
    {KEY_1,      KEY_2,      KEY_3,      KEY_UP},
    {KEY_4,      KEY_5,      KEY_6,      KEY_DOWN},
    {KEY_7,      KEY_8,      KEY_9,      KEY_HASH},
    {KEY_ESC,    KEY_0,      KEY_Confirm,   KEY_Rback}
};

// 添加按键状态变量
static char last_key = KEY_NONE;
static uint32_t last_key_time = 0;

/**
 * @brief 初始化键盘
 */
void Button_Init(void)
{
    /* 将所有行线设置为高电平 */
    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
    last_key = KEY_NONE;
    last_key_time = 0;
}

/**
 * @brief 获取按下的按键（带防抖和防重复）
 */
char Button_GetKey(void)
{
    uint8_t ROW = 0;
    char KeyValue = KEY_NONE;
    uint32_t current_time = HAL_GetTick();
    
    /* 防重复：如果上次按键时间小于200ms，返回无按键 */
    if(last_key != KEY_NONE && (current_time - last_key_time) < 200)
    {
        return KEY_NONE;
    }
    
    /* 将所有行线设为高电平 */
    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
    
    /* 扫描所有行 */
    for(ROW = 0; ROW < 4; ROW++)
    {
        /* 将当前行设为低电平，其他行设为高电平 */
        switch(ROW)
        {
            case 0:
                HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ROW_PORT, ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
                break;
            case 1:
                HAL_GPIO_WritePin(ROW_PORT, ROW2_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
                break;
            case 2:
                HAL_GPIO_WritePin(ROW_PORT, ROW3_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW4_PIN, GPIO_PIN_SET);
                break;
            case 3:
                HAL_GPIO_WritePin(ROW_PORT, ROW4_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN, GPIO_PIN_SET);
                break;
        }
        
        /* 等待电平稳定 */
        HAL_Delay(1);
        
        /* 检测当前行的列输入 */
        if(HAL_GPIO_ReadPin(COL_PORT, COL1_PIN) == GPIO_PIN_RESET)
        {
            KeyValue = key_map[ROW][0];
            break;
        }
        if(HAL_GPIO_ReadPin(COL_PORT, COL2_PIN) == GPIO_PIN_RESET)
        {
            KeyValue = key_map[ROW][1];
            break;
        }
        if(HAL_GPIO_ReadPin(COL_PORT, COL3_PIN) == GPIO_PIN_RESET)
        {
            KeyValue = key_map[ROW][2];
            break;
        }
        if(HAL_GPIO_ReadPin(COL_PORT, COL4_PIN) == GPIO_PIN_RESET)
        {
            KeyValue = key_map[ROW][3];
            break;
        }
    }
    
    /* 恢复所有行线为高电平 */
    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
    
    /* 如果检测到按键，进行防抖处理 */
    if(KeyValue != KEY_NONE)
    {
        /* 防抖延时 */
        HAL_Delay(20);
        
        /* 再次确认按键是否仍然按下（二次检测）*/
        uint8_t confirm = 0;
        for(ROW = 0; ROW < 4; ROW++)
        {
            /* 同样的扫描逻辑 */
            switch(ROW)
            {
                case 0:
                    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(ROW_PORT, ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
                    break;
                case 1:
                    HAL_GPIO_WritePin(ROW_PORT, ROW2_PIN, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
                    break;
                case 2:
                    HAL_GPIO_WritePin(ROW_PORT, ROW3_PIN, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW4_PIN, GPIO_PIN_SET);
                    break;
                case 3:
                    HAL_GPIO_WritePin(ROW_PORT, ROW4_PIN, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN, GPIO_PIN_SET);
                    break;
            }
            HAL_Delay(1);
            
            if(HAL_GPIO_ReadPin(COL_PORT, COL1_PIN) == GPIO_PIN_RESET && key_map[ROW][0] == KeyValue)
            {
                confirm = 1;
                break;
            }
            if(HAL_GPIO_ReadPin(COL_PORT, COL2_PIN) == GPIO_PIN_RESET && key_map[ROW][1] == KeyValue)
            {
                confirm = 1;
                break;
            }
            if(HAL_GPIO_ReadPin(COL_PORT, COL3_PIN) == GPIO_PIN_RESET && key_map[ROW][2] == KeyValue)
            {
                confirm = 1;
                break;
            }
            if(HAL_GPIO_ReadPin(COL_PORT, COL4_PIN) == GPIO_PIN_RESET && key_map[ROW][3] == KeyValue)
            {
                confirm = 1;
                break;
            }
        }
        HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
        
        /* 如果二次确认失败，返回无按键 */
        if(!confirm)
        {
            return KEY_NONE;
        }
        
        /* 等待按键释放 */
        while(1)
        {
            uint8_t released = 1;
            for(ROW = 0; ROW < 4; ROW++)
            {
                switch(ROW)
                {
                    case 0:
                        HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(ROW_PORT, ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
                        break;
                    case 1:
                        HAL_GPIO_WritePin(ROW_PORT, ROW2_PIN, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
                        break;
                    case 2:
                        HAL_GPIO_WritePin(ROW_PORT, ROW3_PIN, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW4_PIN, GPIO_PIN_SET);
                        break;
                    case 3:
                        HAL_GPIO_WritePin(ROW_PORT, ROW4_PIN, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN, GPIO_PIN_SET);
                        break;
                }
                HAL_Delay(1);
                
                if(HAL_GPIO_ReadPin(COL_PORT, COL1_PIN) == GPIO_PIN_RESET ||
                   HAL_GPIO_ReadPin(COL_PORT, COL2_PIN) == GPIO_PIN_RESET ||
                   HAL_GPIO_ReadPin(COL_PORT, COL3_PIN) == GPIO_PIN_RESET ||
                   HAL_GPIO_ReadPin(COL_PORT, COL4_PIN) == GPIO_PIN_RESET)
                {
                    released = 0;
                    break;
                }
            }
            HAL_GPIO_WritePin(ROW_PORT, ROW1_PIN | ROW2_PIN | ROW3_PIN | ROW4_PIN, GPIO_PIN_SET);
            
            if(released)
                break;
                
            HAL_Delay(10);
        }
        
        /* 记录按键和时间 */
        last_key = KeyValue;
        last_key_time = current_time;
        
        return KeyValue;
    }
    
    return KEY_NONE;
}