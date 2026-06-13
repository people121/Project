#ifndef __ADMINOPERATION__H
#define __ADMINOPERATION__H

/*
 *  函数名：Open_Door_Correct
 *  功能描述：操作正确后的开门操作
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Open_Door_Correct(void);
/*
 *  函数名：Open_Door_Error
 *  功能描述：操作错误后的反应状态
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Open_Door_Error(void);
/*
 *  函数名：Uid_Read
 *  功能描述：读取卡片ID并与存储在Flash中的ID进行比较，显示验证结果
 *  输入参数：无
 *  输出参数：无
 *  返回值：0 - 验证成功, 1 - 验证失败
 */
uint8_t Uid_Read(void);
/*
 *  函数名：Regular_User_Menu
 *  功能描述：显示用户界面菜单
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Regular_User_Menu(void);
/*
 *  函数名：Regular_User
 *  功能描述：普通用户界面，显示欢迎信息和操作选项
 *  输入参数：argument - 任务参数（未使用）
 *  输出参数：Key值
 *  返回值：0-成功, 其他值失败
 */
void Regular_User(void);

void Regular_Admin(void);
#endif
