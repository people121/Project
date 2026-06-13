#include "stm32f1xx_hal.h"
#include "button.h"
#include "driver_oled.h"
#include "ESP.h"
#include "FLASH.h"
#include "RC522.h"
#include "DoorOpenPro.h"

#define USER_FLASH_ADDR_PWD              0x08007800          // 扇区30起始地址（30KB位置）用于存储用户密码                  (密码)
#define USER_FLASH_ADDR_RFID             0x08007C00          // 扇区31起始地址（28KB位置）用于存储用户卡片ID                (用户ID)
#define USER_FLASH_ADDR_ADMIN_RFID       0x08008000          // 扇区32起始地址（24KB位置）用于存储管理员卡片ID              (管理员ID)
#define USER_FLASH_ADDR_RFID_New1        0x08008400          // 扇区33起始地址（20KB位置）用于存储新增加的卡片ID1           (新增用户ID1)
#define USER_FLASH_ADDR_RFID_New2        0x08008800          // 扇区34起始地址（16KB位置）用于存储新增加的卡片ID2           (新增用户ID2)
#define USER_FLASH_ADDR_ADMIN_RFID_New1  0x08008C00          // 扇区35起始地址（12KB位置）用于存储新增加的管理员卡片ID1      (新增管理员ID1)
#define USER_FLASH_ADDR_ADMIN_PWD        0x08009000          // 扇区36起始地址（8KB位置）用于存储管理员密码                 (管理员密码)

#define USER_FLASH_ADDR_USER_CARD_COUNT  0x08007FF8          // 用户卡数量地址（扇区31末尾）
#define USER_FLASH_ADDR_ADMIN_CARD_COUNT 0x080083F8          // 管理员卡数量地址（扇区32末尾）

unsigned char user_uid[4];                   // 存储读到的卡片ID
static uint8_t current_menu = 0;             // 0:开门, 1:修改密码, 2:退出

void Regular_Admin(void);
void Change_Password(void);
void Add_ID_Card(void);
void ADD_Admin_ID(void);
void ADD_User_ID(void); 
void Dele_ID_Card(void);
void Dele_User_ID(uint32_t del_card);
void Dele_Admin_ID(uint32_t del_card);
void Admin_Change_Password_operation(void);
void User_Change_Password_operation(void);
/*
 *  函数名：Open_Door_Correct
 *  功能描述：操作正确后的开门操作
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Open_Door_Correct(void)
{
    greenLED_open();            //开启绿灯
    buzzer_open();              //开启蜂鸣器
    open_door();                //开门
    HAL_Delay(500);             //等待1秒钟
    greenLED_close();           //关闭绿灯
    buzzer_close();             //关闭蜂鸣器
    close_door();               //关门
}
/*
 *  函数名：Open_Door_Error
 *  功能描述：操作错误后的反应状态
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Open_Door_Error(void)
{
    redLED_open();              //开启红灯
    buzzer_open();              //开启蜂鸣器
    HAL_Delay(500);            //等待1秒钟
    redLED_close();              //关闭红灯
    buzzer_close();              //关闭蜂鸣器
}
/*
 *  函数名：Uid_Read
 *  功能描述：读取卡片ID并与存储在Flash中的ID进行比较，显示验证结果
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Uid_Read(void)
{
    while(1)
    {
        /* 判断是否读取到卡片 */
        if(RC522_ReadCardID(user_uid) == MI_OK){
            uint32_t current = (user_uid[0] << 24) | (user_uid[1] << 16) | (user_uid[2] << 8) | user_uid[3];
            
            // 1. 检查原始用户卡（扇区31）
            uint32_t read_User = Flash_Read(USER_FLASH_ADDR_RFID);
            
            // 2. 检查新增用户卡1（扇区33）
            uint32_t read_User_New1 = Flash_Read(USER_FLASH_ADDR_RFID_New1);
            
            // 3. 检查新增用户卡2（扇区34）
            uint32_t read_User_New2 = Flash_Read(USER_FLASH_ADDR_RFID_New2);
            
            // 4. 检查原始管理员卡（扇区32）
            uint32_t read_Admin = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID);
            
            // 5. 检查新增管理员卡1（扇区35）
            uint32_t read_Admin_New1 = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID_New1);
            
            /* 判断是否是用户卡 */
            if(read_User == current || read_User_New1 == current || read_User_New2 == current){
                OLED_Clear();
                Open_Door_Correct();
                OLED_PrintString(0, 4, "Card OK ");
                HAL_Delay(1000);
                break;
            }
            /* 判断是否是管理员卡 */
            else if(read_Admin == current || read_Admin_New1 == current){
                OLED_Clear();
                Regular_Admin();
                break;
            }
            else{
                OLED_Clear();
                Open_Door_Error();
                OLED_PrintString(0, 4, "Card error");
                HAL_Delay(1000);
                break;
            }
        }
    }
}
/*
 *  函数名：Regular_User_Menu
 *  功能描述：显示用户界面菜单
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Regular_User_Menu(void)
{
        // 显示普通用户界面
        OLED_PrintString(4, 0, "Welcome");
        OLED_PrintString(0, 2, "Please Open Door");
}
/*
 *  函数名：Regular_User
 *  功能描述：普通用户界面，显示欢迎信息和操作选项
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Regular_User(void)
{
    char count = 0;                                 // 记录输入密码的长度
    char password[7] = {0};                         // 存储输入的密码，最后一个字符为'\0'
    char display_char[2] = {0};                     // 用于显示单个字符的字符串

    OLED_Clear();
    Regular_User_Menu();                            // 调用显示普通用户界面函数
    HAL_Delay(1000);                                // 等待1秒钟显示欢迎信息
    OLED_Clear();
    OLED_PrintString(0, 2, "Password:");

    while (1)
    {
		// 处理ESP接收到的数据（开门指令）
        ESP_Process();
		
        char key = Button_GetKey();                 // 获取键盘输入    
        if(RC522_ReadCardID(user_uid) == MI_OK){    // 如果检测到卡片，读取卡片ID
            Uid_Read();                             
        }

        // 如果检测到0-9的按键,并且输入的密码长度小于6，则将按键值存储到password数组中，并显示在OLED上
        if ((key >= '0' && key <= '9')&&(count < 6)){
            // 存储密码
            password[count] = key;
            // 将字符转换为字符串并显示
            display_char[0] = key;
            display_char[1] = '\0';
            OLED_PrintString(9 + count, 2, display_char);
            count++;
            HAL_Delay(200); // 防止按键抖动
        /*如果检测到按键时确认键*/
        }else if (key == KEY_Confirm){
            // 处理确认键，验证密码
            int input_pwd = 0;
            for(int i = 0; i < count; i++) {
                input_pwd = input_pwd * 10 + (password[i] - '0');
            }
            uint32_t pwd_admin= Flash_Read(USER_FLASH_ADDR_ADMIN_PWD); // 从Flash读取存储的管理员密码进行验证
			uint32_t pwd_user= Flash_Read(USER_FLASH_ADDR_PWD); // 从Flash读取存储的用户密码进行验证
            if(pwd_user == input_pwd){
                OLED_Clear();
                OLED_PrintString(3, 2, "Door Opened");
                HAL_Delay(500);                             // 显示开门成功信息2秒
                Open_Door_Correct();                        //模拟开门操作
                OLED_Clear();
            }else if(pwd_admin == input_pwd){
                OLED_Clear();
                OLED_PrintString(3, 2, "Admin Door Opened");
                HAL_Delay(500);                             // 显示开门成功信息2秒
                Regular_Admin();                            //进入管理员界面
                OLED_Clear();
            }else{
                OLED_Clear();
                OLED_PrintString(3, 2, "Password Error");   
                HAL_Delay(500); // 显示密码错误信息2秒
                Open_Door_Error();                          //模拟关门操作
                OLED_Clear();
            }
        }else if (key == KEY_ESC){
			Regular_User();
        }else if (key == KEY_Rback){
            // 处理回退键，删除最后一个输入的字符
            if (count > 0){
                count--;
                password[count] = '\0'; // 删除最后一个字符
                OLED_PrintString(count + 9, 2, " "); // 清除对应位置的*
            }
        }
    }
}
/*
 *  函数名：Regular_Admin
 *  功能描述：管理员界面，显示管理员操作选项
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Regular_Admin(void)
{
    static uint8_t Point_To = 2;                           //显示"<"的位置
    // 显示管理员界面
    OLED_Clear();
    OLED_PrintString(3, 0, "Admin Menu");           // 显示管理员界面标题
    OLED_PrintString(1, 2, "1. Change Pwd");        // 修改密码
    OLED_PrintString(1, 4, "2. Add ID");            // 增加ID卡
    OLED_PrintString(1, 6, "3. Del ID");            // 删除ID卡
    while (1){
        char key = Button_GetKey();                 // 获取键盘输入   
        if(KEY_UP == key && Point_To > 2){
            OLED_PrintString(0, Point_To, " ");     // 清除当前位置
            Point_To -= 2;                          // 移动指针
            OLED_PrintString(0, Point_To, ">");     // 显示新位置
        }else if (KEY_DOWN == key && Point_To < 6){
            OLED_PrintString(0, Point_To, " ");      // 清除当前位置
            Point_To += 2;                           // 移动指针
            OLED_PrintString(0, Point_To, ">");      // 显示新位置
        }else if (KEY_Confirm == key){
            switch (Point_To){
                case 2:
                    //修改密码
                    Change_Password();
                    break;
                
                case 4:
                    //增加ID卡
                    Add_ID_Card();
                    break;
                case 6:
                    //删除ID卡
                    Dele_ID_Card();
                    break;
            }
        }else if (KEY_ESC == key){
            Regular_User();                         //切换到普通用户
        }
    }
}
/*
 *  函数名：Change_Password
 *  功能描述：修改密码操作
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Change_Password(void)
{   
    char ID_Select = 2;
    OLED_Clear();
    OLED_PrintString(0,3,"Change password");
    HAL_Delay(1000);
    OLED_Clear();
    while(1){
        OLED_PrintString(0,0,"Change password");
        OLED_PrintString(1,2,"1-User_PWD");
        OLED_PrintString(1,4,"2-Admin_PWD");
        char key = Button_GetKey();                 // 获取键盘输入   
        if(KEY_UP == key && ID_Select > 2){
            OLED_PrintString(0, ID_Select, " ");     // 清除当前位置
            ID_Select -= 2;                          // 移动指针
            OLED_PrintString(0, ID_Select, ">");     // 显示新位置
        }else if (KEY_DOWN == key && ID_Select < 4){
            OLED_PrintString(0, ID_Select, " ");      // 清除当前位置
            ID_Select += 2;                           // 移动指针
            OLED_PrintString(0, ID_Select, ">");      // 显示新位置
        }else if (KEY_Confirm == key){
            switch (ID_Select){
            case  2:
                User_Change_Password_operation();
                break;
            default:
                Admin_Change_Password_operation();
                break;
            }
        }else if (KEY_ESC == key){
            break;
        }
    }
}
void User_Change_Password_operation(void)
{
    char count[3] = {0, 0, 0};
    char password[3][7] = {{0}};
    char display_char[2] = {0};                     
    static uint8_t current_line = 0;
    static uint8_t pos_y[3] = {2, 4, 6};

    uint32_t User_Old_Pwd = 0;
    uint32_t User_Old_Flash = 0;
    uint32_t new_pwd_value = 0;
    uint32_t con_pwd_value = 0;
    
    OLED_Clear();
    OLED_PrintString(2, 0, "Change User Pwd");
    OLED_PrintString(0, 2, "Old_Pwd:");
    OLED_PrintString(0, 4, "New_Pwd:");
    OLED_PrintString(0, 6, "Con_Pwd:");
    
    User_Old_Flash = Flash_Read(USER_FLASH_ADDR_PWD);
    // 显示初始光标
    OLED_PrintString(8, pos_y[current_line], "_");
    
    while(1){
        char key = Button_GetKey();
        
        if ((key >= '0' && key <= '9') && (count[current_line] < 6)){
            OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
            password[current_line][count[current_line]] = key;
            display_char[0] = key;
            display_char[1] = '\0';
            OLED_PrintString(8 + count[current_line], pos_y[current_line], display_char);
            count[current_line]++;
            if(count[current_line] < 6){
                OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
            }
            HAL_Delay(200);
        }
        else if (key == KEY_Rback){
            if (count[current_line] > 0){
                count[current_line]--;
                password[current_line][count[current_line]] = '\0';
                OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
                OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
                HAL_Delay(200);
            }
        }
        else if (KEY_UP == key && current_line > 0){
            OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
            current_line--;
            OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
            HAL_Delay(200);
        }
        else if (KEY_DOWN == key && current_line < 2){
            OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
            current_line++;
            OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
            HAL_Delay(200);
        }
        else if (KEY_ESC == key){
            Regular_Admin();
            return;
        }
        else if (KEY_Confirm == key){
            // 获取旧密码
            for(int i = 0; i < count[0]; i++) {
                User_Old_Pwd = User_Old_Pwd * 10 + (password[0][i] - '0');
            }
            // 获取新密码
            for(int i = 0; i < count[1]; i++) {
                new_pwd_value = new_pwd_value * 10 + (password[1][i] - '0');
            }
            // 获取确认密码
            for(int i = 0; i < count[2]; i++) {
                con_pwd_value = con_pwd_value * 10 + (password[2][i] - '0');
            }
            
            // 验证：旧密码正确 && 新密码与确认密码一致 && 新密码不等于旧密码
            if((User_Old_Pwd == User_Old_Flash) && (new_pwd_value == con_pwd_value) && (User_Old_Pwd != new_pwd_value)){
                OLED_Clear();
                OLED_PrintString(0, 3, "User Pwd Changed!");
                Flash_Erase(USER_FLASH_ADDR_PWD);
                HAL_Delay(10);
                Flash_Write(USER_FLASH_ADDR_PWD, con_pwd_value);
                HAL_Delay(1000);
                Regular_Admin();
                return;
            } else {
                OLED_Clear();
                OLED_PrintString(0, 3, "Password Error!");
                HAL_Delay(1000);
                Regular_Admin();
                return;
            }
        }
    }
}

void Admin_Change_Password_operation(void)
{
    char count[3] = {0, 0, 0};
    char password[3][7] = {{0}};
    char display_char[2] = {0};                     
    static uint8_t current_line = 0;
    static uint8_t pos_y[3] = {2, 4, 6};

    uint32_t Admin_Old_Pwd = 0;
    uint32_t Admin_Old_Flash = 0;
    uint32_t new_pwd_value = 0;
    uint32_t con_pwd_value = 0;
    
    OLED_Clear();
    OLED_PrintString(2, 0, "Change Admin Pwd");
    OLED_PrintString(0, 2, "Old_Pwd:");
    OLED_PrintString(0, 4, "New_Pwd:");
    OLED_PrintString(0, 6, "Con_Pwd:");
    
    Admin_Old_Flash = Flash_Read(USER_FLASH_ADDR_ADMIN_PWD);
    // 显示初始光标
    OLED_PrintString(8, pos_y[current_line], "_");
    
    while(1){
        char key = Button_GetKey();
        
        if ((key >= '0' && key <= '9') && (count[current_line] < 6)){
            OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
            password[current_line][count[current_line]] = key;
            display_char[0] = key;
            display_char[1] = '\0';
            OLED_PrintString(8 + count[current_line], pos_y[current_line], display_char);
            count[current_line]++;
            if(count[current_line] < 6){
                OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
            }
            HAL_Delay(200);
        }
        else if (key == KEY_Rback){
            if (count[current_line] > 0){
                count[current_line]--;
                password[current_line][count[current_line]] = '\0';
                OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
                OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
                HAL_Delay(200);
            }
        }
        else if (KEY_UP == key && current_line > 0){
            OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
            current_line--;
            OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
            HAL_Delay(200);
        }
        else if (KEY_DOWN == key && current_line < 2){
            OLED_PrintString(8 + count[current_line], pos_y[current_line], " ");
            current_line++;
            OLED_PrintString(8 + count[current_line], pos_y[current_line], "_");
            HAL_Delay(200);
        }
        else if (KEY_ESC == key){
            Regular_Admin();
            return;
        }
        else if (KEY_Confirm == key){
            // 获取旧密码
            for(int i = 0; i < count[0]; i++) {
                Admin_Old_Pwd = Admin_Old_Pwd * 10 + (password[0][i] - '0');
            }
            // 获取新密码
            for(int i = 0; i < count[1]; i++) {
                new_pwd_value = new_pwd_value * 10 + (password[1][i] - '0');
            }
            // 获取确认密码
            for(int i = 0; i < count[2]; i++) {
                con_pwd_value = con_pwd_value * 10 + (password[2][i] - '0');
            }
            
            // 验证：旧密码正确 && 新密码与确认密码一致 && 新密码不等于旧密码
            if((Admin_Old_Pwd == Admin_Old_Flash) && (new_pwd_value == con_pwd_value) && (Admin_Old_Pwd != new_pwd_value)){
                OLED_Clear();
                OLED_PrintString(0, 3, "Admin Pwd Changed!");
                Flash_Erase(USER_FLASH_ADDR_ADMIN_PWD);
                HAL_Delay(10);
                Flash_Write(USER_FLASH_ADDR_ADMIN_PWD, con_pwd_value);
                HAL_Delay(1000);
                Regular_Admin();
                return;
            } else {
                OLED_Clear();
                OLED_PrintString(0, 3, "Password Error!");
                HAL_Delay(1000);
                Regular_Admin();
                return;
            }
        }
    }
}

/*
 *  函数名：Add_ID_Card
 *  功能描述：增加卡片操作
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Add_ID_Card(void)
{
    char ID_Select = 2;
    OLED_Clear();
    OLED_PrintString(0,3,"Please swipe new card");
    HAL_Delay(1000);
    while (1){      
        if(RC522_ReadCardID(user_uid) == MI_OK){
        uint32_t current = (user_uid[0] << 24) | (user_uid[1] << 16) | (user_uid[2] << 8) | user_uid[3];
        uint32_t read_User_ID = Flash_Read(USER_FLASH_ADDR_RFID);                                                       // 从FLASH中读取存储的卡片ID进行验证
        uint32_t read_Admin_ID = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID);                                                 // 从FLASH中读取存储的管理员卡片ID进行验证
        uint32_t read_User_ID_new1 =  Flash_Read(USER_FLASH_ADDR_RFID_New1);
		uint32_t read_User_ID_new2 =  Flash_Read(USER_FLASH_ADDR_RFID_New2);
		uint32_t read_Admin_ID_new = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID_New1);
			if (current == read_User_ID || current == read_Admin_ID || current == read_User_ID_new1 ||
				current == read_User_ID_new2 || current == read_Admin_ID_new){
                OLED_Clear();
                OLED_PrintString(3,3,"ID Card Repeat");
                HAL_Delay(1000);
                break;
            }else{
                //将刷到的新卡片加入到Flash中
                OLED_Clear();
                OLED_PrintString(0,0,"Select ID Object");
                OLED_PrintString(1,2,"1-User_ID");
                OLED_PrintString(1,4,"2-Admin_ID");
				while(1){
					char key = Button_GetKey();                 // 获取键盘输入   
					if(KEY_UP == key && ID_Select == 4){
						OLED_PrintString(0, ID_Select, " ");     // 清除当前位置
						ID_Select -= 2;                          // 移动指针
						OLED_PrintString(0, ID_Select, ">");     // 显示新位置
					}else if (KEY_DOWN == key && ID_Select == 2){
						OLED_PrintString(0, ID_Select, " ");      // 清除当前位置
						ID_Select += 2;                           // 移动指针
						OLED_PrintString(0, ID_Select, ">");      // 显示新位置
					}else if (KEY_Confirm == key){
						switch (ID_Select){
						case  2:
							ADD_User_ID();
							break;
						default:
							ADD_Admin_ID();
							break;
						}
					}else if (KEY_ESC == key){
						Regular_Admin();
					}
				}
            }
        }
    }
}

/*
 *  函数名：ADD_User_ID
 *  功能描述：添加用户卡片（每个扇区只存一张卡）
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void ADD_User_ID(void) 
{
    uint32_t current = (user_uid[0] << 24) | (user_uid[1] << 16) | (user_uid[2] << 8) | user_uid[3];
    
    // 1. 检查是否与原始用户卡重复
    uint32_t original_user = Flash_Read(USER_FLASH_ADDR_RFID);
    if (original_user == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 2. 检查是否与原始管理员卡重复
    uint32_t original_admin = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID);
    if (original_admin == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 3. 检查扇区33（用户卡1）
    uint32_t card1 = Flash_Read(USER_FLASH_ADDR_RFID_New1);
    if (card1 == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 4. 检查扇区34（用户卡2）
    uint32_t card2 = Flash_Read(USER_FLASH_ADDR_RFID_New2);
    if (card2 == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 5. 检查扇区35（管理员卡1）
    uint32_t admin_card1 = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID_New1);
    if (admin_card1 == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 检查通过，开始添加
    // 检查扇区33是否为空
    if (card1 == 0xFFFFFFFF) {
        Flash_Erase(USER_FLASH_ADDR_RFID_New1);
        HAL_Delay(10);
        Flash_Write(USER_FLASH_ADDR_RFID_New1, current);
        OLED_Clear();
        OLED_PrintString(0, 3, "User Card1 Added!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 检查扇区34是否为空
    if (card2 == 0xFFFFFFFF) {
        Flash_Erase(USER_FLASH_ADDR_RFID_New2);
        HAL_Delay(10);
        Flash_Write(USER_FLASH_ADDR_RFID_New2, current);
        OLED_Clear();
        OLED_PrintString(0, 3, "User Card2 Added!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 所有卡槽都已满
    OLED_Clear();
    OLED_PrintString(0, 3, "User Card Full!");
    HAL_Delay(1000);
    Regular_Admin();
}

/*
 *  函数名：ADD_Admin_ID
 *  功能描述：添加管理员卡片（每个扇区只存一张卡）
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void ADD_Admin_ID(void)
{
    uint32_t current = (user_uid[0] << 24) | (user_uid[1] << 16) | (user_uid[2] << 8) | user_uid[3];
    
    // 1. 检查是否与原始用户卡重复
    uint32_t original_user = Flash_Read(USER_FLASH_ADDR_RFID);
    if (original_user == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 2. 检查是否与原始管理员卡重复
    uint32_t original_admin = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID);
    if (original_admin == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 3. 检查扇区33（用户卡1）
    uint32_t user_card1 = Flash_Read(USER_FLASH_ADDR_RFID_New1);
    if (user_card1 == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 4. 检查扇区34（用户卡2）
    uint32_t user_card2 = Flash_Read(USER_FLASH_ADDR_RFID_New2);
    if (user_card2 == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 5. 检查扇区35（管理员卡1）
    uint32_t admin_card1 = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID_New1);
    if (admin_card1 == current) {
        OLED_Clear();
        OLED_PrintString(0, 3, "Card already exists!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 检查通过，开始添加
    // 检查扇区35是否为空
    if (admin_card1 == 0xFFFFFFFF) {
        Flash_Erase(USER_FLASH_ADDR_ADMIN_RFID_New1);
        HAL_Delay(10);
        Flash_Write(USER_FLASH_ADDR_ADMIN_RFID_New1, current);
        OLED_Clear();
        OLED_PrintString(0, 3, "Admin Card Added!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 所有卡槽都已满
    OLED_Clear();
    OLED_PrintString(0, 3, "Admin Card Full!");
    HAL_Delay(1000);
    Regular_Admin();
}

/*
 *  函数名：Dele_ID_Card
 *  功能描述：删除ID卡操作 - 显示所有卡片供选择删除
 *  输入参数：无
 *  输出参数：无
 *  返回值：无
 */
void Dele_ID_Card(void)
{
    // 收集所有卡片信息
    typedef struct {
        uint32_t card_id;
        uint8_t type;  // 0:用户卡(U), 1:管理员卡(A)
        uint8_t slot;  // 卡槽位置
        uint8_t can_delete;  // 是否可删除
    } CardInfo;
    
    CardInfo card_list[10];
    uint8_t card_count = 0;
    
    // 1. 原始用户卡（扇区31）- 不可删除
    uint32_t card = Flash_Read(USER_FLASH_ADDR_RFID);
    if (card != 0xFFFFFFFF) {
        card_list[card_count].card_id = card;
        card_list[card_count].type = 0;
        card_list[card_count].slot = 0;
        card_list[card_count].can_delete = 0;
        card_count++;
    }
    
    // 2. 新增用户卡1（扇区33）
    card = Flash_Read(USER_FLASH_ADDR_RFID_New1);
    if (card != 0xFFFFFFFF) {
        card_list[card_count].card_id = card;
        card_list[card_count].type = 0;
        card_list[card_count].slot = 1;
        card_list[card_count].can_delete = 1;
        card_count++;
    }
    
    // 3. 新增用户卡2（扇区34）
    card = Flash_Read(USER_FLASH_ADDR_RFID_New2);
    if (card != 0xFFFFFFFF) {
        card_list[card_count].card_id = card;
        card_list[card_count].type = 0;
        card_list[card_count].slot = 2;
        card_list[card_count].can_delete = 1;
        card_count++;
    }
    
    // 4. 原始管理员卡（扇区32）- 不可删除
    card = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID);
    if (card != 0xFFFFFFFF) {
        card_list[card_count].card_id = card;
        card_list[card_count].type = 1;
        card_list[card_count].slot = 0;
        card_list[card_count].can_delete = 0;
        card_count++;
    }
    
    // 5. 新增管理员卡（扇区35）
    card = Flash_Read(USER_FLASH_ADDR_ADMIN_RFID_New1);
    if (card != 0xFFFFFFFF) {
        card_list[card_count].card_id = card;
        card_list[card_count].type = 1;
        card_list[card_count].slot = 1;
        card_list[card_count].can_delete = 1;
        card_count++;
    }
    
    if (card_count == 0) {
        OLED_Clear();
        OLED_PrintString(2, 3, "No Card Found!");
        HAL_Delay(1000);
        Regular_Admin();
        return;
    }
    
    // 显示卡片列表供选择（每页3个，行号：2,4,6）
    uint8_t current_page = 0;
    uint8_t total_pages = (card_count - 1) / 3 + 1;
    uint8_t current_selection = 0;
    char display_buf[30];
    
    // 初始绘制一次
    uint8_t need_refresh = 1;
    uint8_t last_selection = 0xFF;
    uint8_t last_page = 0xFF;
    
    while (1) {
        // 只在需要时刷新屏幕
        if (need_refresh || current_selection != last_selection || current_page != last_page) {
            last_selection = current_selection;
            last_page = current_page;
            
            OLED_Clear();
            
            // 显示标题
            OLED_PrintString(2, 0, "Delete ID Card");
            
            // 显示当前页的卡片（每页3个，行号：2,4,6）
            uint8_t start_idx = current_page * 3;
            uint8_t end_idx = start_idx + 3;
            if (end_idx > card_count) end_idx = card_count;
            
            for (int i = start_idx; i < end_idx; i++) {
                uint8_t line = 2 + (i - start_idx) * 2;
                
                // 显示选择指针
                if (i == current_selection) {
                    OLED_PrintString(0, line, ">");
                } else {
                    OLED_PrintString(0, line, " ");
                }
                
                // 显示卡片ID和类型
                uint32_t show_id = card_list[i].card_id;
                if (card_list[i].type == 0) {
                    sprintf(display_buf, "%08lX U", show_id);
                } else {
                    sprintf(display_buf, "%08lX A", show_id);
                }
                OLED_PrintString(2, line, display_buf);
            }
            
            need_refresh = 0;
        }
        
        // 处理按键
        char key = Button_GetKey();
        
        if (KEY_UP == key) {
            if (current_selection > 0) {
                current_selection--;
                if (current_selection < current_page * 3) {
                    current_page--;
                }
                need_refresh = 1;
            }
            HAL_Delay(150);
        }
        else if (KEY_DOWN == key) {
            if (current_selection < card_count - 1) {
                current_selection++;
                if (current_selection >= (current_page + 1) * 3) {
                    current_page++;
                }
                need_refresh = 1;
            }
            HAL_Delay(150);
        }
        else if (KEY_Confirm == key) {
            // 检查是否可删除
            if (!card_list[current_selection].can_delete) {
                OLED_Clear();
                OLED_PrintString(0, 3, "Cannot delete fixed card!");
                HAL_Delay(1000);
                need_refresh = 1;
                continue;
            }
            
            // 第一次确认
            OLED_Clear();
            OLED_PrintString(2, 2, "Confirm deletion?");
            
            uint8_t confirmed = 0;
            uint32_t confirm_time = HAL_GetTick();
            
            while (1) {
                char confirm_key = Button_GetKey();
                
                if (KEY_Confirm == confirm_key) {
                    confirmed = 1;
                    break;
                }
                else if (KEY_ESC == confirm_key) {
                    confirmed = 0;
                    break;
                }
                
                // 3秒超时自动取消
                if (HAL_GetTick() - confirm_time > 3000) {
                    confirmed = 0;
                    break;
                }
                HAL_Delay(50);
            }
            
            if (confirmed) {
                // 执行删除
                uint32_t del_card = card_list[current_selection].card_id;
                uint8_t del_type = card_list[current_selection].type;
                uint8_t del_slot = card_list[current_selection].slot;
                
                if (del_type == 0) {
                    if (del_slot == 1) {
                        Flash_Erase(USER_FLASH_ADDR_RFID_New1);
                    } else if (del_slot == 2) {
                        Flash_Erase(USER_FLASH_ADDR_RFID_New2);
                    }
                } else {
                    if (del_slot == 1) {
                        Flash_Erase(USER_FLASH_ADDR_ADMIN_RFID_New1);
                    }
                }
                
                OLED_Clear();
                OLED_PrintString(2, 3, "Card Deleted!");
                HAL_Delay(1000);
                Regular_Admin();
                return;
            }
            // 取消删除，刷新列表
            need_refresh = 1;
        }
        else if (KEY_ESC == key) {
            Regular_Admin();
            return;
        }
        
        HAL_Delay(20);
    }
}