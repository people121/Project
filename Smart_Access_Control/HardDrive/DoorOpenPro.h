#ifndef __DOOROPENPRO_H
#define __DOOROPENPRO_H

/*
 *  函数名：buzzer_open
 *  功能描述：将蜂鸣器打开
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void buzzer_open(void);
/*
 *  函数名：buzzer_close
 *  功能描述：将蜂鸣器关闭
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void buzzer_close(void);
/*
 *  函数名：redLED_open
 *  功能描述：将红灯打开
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void redLED_open(void);
/*
 *  函数名：redLED_close 
 *  功能描述：将红灯关闭
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void redLED_close(void);

/*
 *  函数名：greenLED_open
 *  功能描述：将绿灯打开
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void greenLED_open(void);
/*
 *  函数名：greenLED_close
 *  功能描述：将绿灯关闭
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void greenLED_close(void);

/*
 *  函数名：PWM_SetCompare1
 *  功能描述：设置TIM1通道1的比较值，控制PWM占空比
 *  输入参数：Compare --> 比较值，范围0~2000（对应0~100%占空比）
 *  输出参数：无
 *  返回值：无
 */
void PWM_SetCompare1(uint16_t Compare);
/*
 *  函数名：Servo_SetAngle
 *  功能描述：设置舵机的角度，0度对应500us脉宽，180度对应2500us脉宽
 *  输入参数：Angle --> 角度值，范围0~180   
 *  输出参数：无
 *  返回值：无
 */
void Servo_SetAngle(float Angle);
/*
 *  函数名：open_door
 *  功能描述：打开门（将舵机转到90度位置）
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void open_door(void);
/*
 *  函数名：close_door
 *  功能描述：关闭门（将舵机转到0度位置）
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void close_door(void);


#endif