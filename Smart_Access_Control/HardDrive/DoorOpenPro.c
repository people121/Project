#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "DoorOpenPro.h"
#include "tim.h"

extern TIM_HandleTypeDef htim1;
/*
 *  函数名：buzzer_open
 *  功能描述：将蜂鸣器打开
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void buzzer_open(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}
/*
 *  函数名：buzzer_close
 *  功能描述：将蜂鸣器关闭
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void buzzer_close(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
}
/*
 *  函数名：redLED_open
 *  功能描述：将红灯打开
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void redLED_open(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
}
/*
 *  函数名：redLED_close 
 *  功能描述：将红灯关闭
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void redLED_close(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
}

/*
 *  函数名：greenLED_open
 *  功能描述：将绿灯打开
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void greenLED_open(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}
/*
 *  函数名：greenLED_close
 *  功能描述：将绿灯关闭
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void greenLED_close(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
}

/*
 *  函数名：PWM_SetCompare1
 *  功能描述：设置TIM1通道1的比较值，控制PWM占空比
 *  输入参数：Compare --> 比较值，范围500~2500（对应0~180度）
 *  输出参数：无
 *  返回值：无
 */
void PWM_SetCompare1(uint16_t Compare)
{
    // 限制范围，保护舵机
    if(Compare < 500) Compare = 500;
    if(Compare > 2500) Compare = 2500;
    
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Compare);
}

/*
 *  函数名：Servo_SetAngle
 *  功能描述：设置舵机的角度
 *  输入参数：Angle --> 角度值，范围0~180
 *  输出参数：无
 *  返回值：无
 */
void Servo_SetAngle(float Angle)
{
    // 角度限制
    if(Angle < 0) Angle = 0;
    if(Angle > 180) Angle = 180;
    
    // 脉宽范围：500us（0度）~ 2500us（180度）
    // 计数范围：500 ~ 2500
    uint16_t compare = 500 + (uint16_t)(Angle * 2000.0f / 180.0f);
    
    PWM_SetCompare1(compare);
}

/*
 *  函数名：open_door
 *  功能描述：打开门（将舵机转到90度位置）
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void open_door(void)
{
    Servo_SetAngle(90);  // 90度开门
}

/*
 *  函数名：close_door
 *  功能描述：关闭门（将舵机转到0度位置）
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void close_door(void)
{
    Servo_SetAngle(0);   // 0度关门
}