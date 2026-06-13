根据你提供的完整代码，我为你生成了一份清晰的 **MD文件说明**，详细介绍了这个基于 STM32 的智能门禁系统的项目结构、功能模块和使用方法。

```markdown
# 智能门禁系统 - Smart Access Control

## ? 项目简介

本项目是一个基于 **STM32F103** 微控制器的智能门禁系统，实现了多种身份验证方式（密码、RFID 卡片、远程控制），并具备管理员管理功能。系统采用模块化设计，包含以下核心功能：
- 密码开门（用户密码/管理员密码）
- RFID 卡片开门（普通用户卡/管理员卡）
- 远程控制开门（通过 ESP-01S 连接巴法云平台）
- 管理员功能：修改密码、添加/删除卡片

## ? 硬件平台

| 模块 | 型号/引脚 | 说明 |
|------|-----------|------|
| 主控芯片 | STM32F103 | Cortex-M3内核 |
| RFID模块 | RC522 | SPI接口（PA4~PA7） |
| OLED显示屏 | 0.96寸 I2C | 显示交互界面 |
| 矩阵键盘 | 4x4 | 密码输入/菜单选择 |
| 舵机 | SG90 | 控制门锁（PWM） |
| Wi-Fi模块 | ESP-01S | 连接巴法云平台 |
| LED指示 | 红灯/绿灯 | 状态反馈 |
| 蜂鸣器 | 有源蜂鸣器 | 操作提示音 |

## ? 项目文件结构
```

Smart Access Control/
├── Core/                           # 核心代码
│   ├── main.c                      # 主程序入口
│   ├── AdminOperation.c/h          # 管理员操作（密码修改/卡片管理）
│   ├── Button.c/h                  # 4x4矩阵键盘驱动
│   ├── DoorOpenPro.c/h             # 开门控制（舵机/蜂鸣器/LED）
│   ├── driver_oled.c/h             # OLED显示屏驱动
│   ├── ESP.c/h                     # ESP-01S Wi-Fi模块驱动
│   ├── FLASH.c/h                   # 内部Flash读写（存储密码/卡片）
│   ├── RC522.c/h                   # RC522 RFID模块驱动
│   ├── MySPI.c/h                   # 软件SPI通信
│   ├── RC522_Ins.h                 # RC522寄存器/指令定义
│   └── ascii_font.c                # OLED ASCII字库
├── HAL/                            # STM32 HAL库配置
│   ├── gpio.c/h                    # GPIO初始化
│   ├── i2c.c/h                     # I2C初始化（OLED）
│   ├── spi.c/h                     # SPI初始化（RC522）
│   ├── tim.c/h                     # 定时器初始化（PWM舵机）
│   ├── usart.c/h                   # 串口初始化（ESP-01S）
│   └── dma.c/h                     # DMA配置（串口接收）
└── startup/                        # 启动文件
    ├── stm32f1xx_it.c              # 中断服务函数
    └── system_stm32f1xx.c          # 系统时钟配置

```
## ?? 功能模块详解

### 1. 主程序流程 (`main.c`)

```c
// 系统初始化
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_I2C1_Init();      // OLED
MX_SPI1_Init();      // RC522
MX_TIM1_Init();      // 舵机PWM
MX_USART1_UART_Init(); // ESP-01S

// 外设初始化
OLED_Init();
RC522_Init();
Button_Init();

// ESP-01S连接巴法云
ESP_Init();

// 从Flash读取或初始化默认数据（密码、卡片ID）
if(init_flag != 0x5A5A5A5A) {
    Flash_Write(USER_FLASH_ADDR_PWD, 123456);           // 用户密码
    Flash_Write(USER_FLASH_ADDR_ADMIN_PWD, 000000);     // 管理员密码
    Flash_Write(USER_FLASH_ADDR_RFID, 0xBAD3411D);      // 用户卡ID
    Flash_Write(USER_FLASH_ADDR_ADMIN_RFID, 0xC7F57B33); // 管理员卡ID
    Flash_Write(USER_FLASH_ADDR_INIT_FLAG, 0x5A5A5A5A);
}

// 进入普通用户界面
Regular_User();

// 主循环：ESP消息处理 + 心跳包
while(1) {
    ESP_Process();
    // 每30秒发送心跳
}
```

### 2. 普通用户界面 (`AdminOperation.c`)

| 验证方式 | 操作说明                        |
| ---- | --------------------------- |
| 密码开门 | 输入6位密码，按 `KEY_Confirm` 确认   |
| 卡片开门 | 刷卡（RC522）自动识别               |
| 远程开门 | 手机发送"on"命令到巴法云，ESP-01S接收后开门 |

**开门成功**：绿灯亮 + 蜂鸣器响 + 舵机转90° → 1秒后自动关门  
**开门失败**：红灯亮 + 蜂鸣器响

### 3. 管理员界面

管理员刷卡或输入管理员密码后进入，功能包括：

| 功能             | 说明                               |
| -------------- | -------------------------------- |
| **Change Pwd** | 修改用户密码或管理员密码（需验证旧密码）             |
| **Add ID**     | 添加新卡片（用户卡/管理员卡），最多2张用户卡 + 1张管理员卡 |
| **Del ID**     | 删除已添加的卡片（原始卡片不可删除）               |

### 4. 存储方案 (FLASH)

| 地址         | 存储内容   | 大小  |
| ---------- | ------ | --- |
| 0x08007800 | 用户密码   | 32位 |
| 0x08007C00 | 用户卡ID  | 32位 |
| 0x08008000 | 管理员卡ID | 32位 |
| 0x08008400 | 新增用户卡1 | 32位 |
| 0x08008800 | 新增用户卡2 | 32位 |
| 0x08008C00 | 新增管理员卡 | 32位 |
| 0x08009000 | 管理员密码  | 32位 |
| 0x08009400 | 初始化标志  | 32位 |

> 每张卡片ID为4字节，存储在单独扇区，便于管理。

### 5. 远程控制 (ESP-01S + 巴法云)

```c
// 巴法云配置
char uid[] = "997fb7889ff742939fe5860e41a201d1";
char topic[] = "led001";

// 连接巴法云 → 订阅主题 → 接收消息
ESP_Init();

// 收到消息后解析
if (strcmp(msg, "on") == 0) {
    Open_Door_Correct();   // 开门
} else if (strcmp(msg, "off") == 0) {
    Open_Door_Error();     // 错误提示（红灯+蜂鸣）
}
```

### 6. 舵机控制 (PWM)

- **频率**：50Hz（周期20ms）
- **占空比范围**：500us（0°）~ 2500us（180°）
- **开门角度**：90°（约1500us）

```c
void open_door(void)  { Servo_SetAngle(90); }  // 开门
void close_door(void) { Servo_SetAngle(0); }   // 关门
```

## ? 引脚连接表

| 外设         | 引脚              | 功能        |
| ---------- | --------------- | --------- |
| RC522_CS   | PA4             | 片选        |
| RC522_SCK  | PA5             | 时钟        |
| RC522_MISO | PA6             | 主机输入      |
| RC522_MOSI | PA7             | 主机输出      |
| RC522_RST  | PA11            | 复位        |
| OLED_SCL   | PB6             | I2C时钟     |
| OLED_SDA   | PB7             | I2C数据     |
| 蜂鸣器        | PA12            | 有源蜂鸣器     |
| 红灯         | PB8             | 错误指示      |
| 绿灯         | PB9             | 成功指示      |
| 舵机PWM      | PA8             | TIM1_CH1  |
| ESP_TX     | PA9             | USART1_TX |
| ESP_RX     | PA10            | USART1_RX |
| 键盘行        | PA0~PA3         | 行扫描输出     |
| 键盘列        | PB0,PB1,PB3,PB4 | 列扫描输入     |

## ? 4x4矩阵键盘映射

| 按键  | 功能  | 按键  | 功能    |
| --- | --- | --- | ----- |
| 1   | 数字1 | ↑   | 上移    |
| 2   | 数字2 | ↓   | 下移    |
| 3   | 数字3 | #   | 保留    |
| 4   | 数字4 | ESC | 返回/退出 |
| 5   | 数字5 | 0   | 数字0   |
| 6   | 数字6 | C   | 确认    |
| 7   | 数字7 | R   | 回退/删除 |
| 8   | 数字8 |     |       |
| 9   | 数字9 |     |       |

## ? 编译与烧录

1. **开发环境**：Keil MDK 或 STM32CubeIDE
2. **烧录工具**：ST-Link / J-Link
3. **烧录步骤**：
   - 编译工程生成 `.hex` 文件
   - 连接ST-Link到SWD接口（PA13、PA14）
   - 使用STM32CubeProgrammer烧录

## ? 使用说明

### 首次上电

- 系统自动初始化默认数据：
  - 用户密码：`123456`
  - 管理员密码：`000000`
  - 用户卡ID：`0xBAD3411D`
  - 管理员卡ID：`0xC7F57B33`

### 普通用户开门

1. **密码开门**：输入6位密码 → 按 `C` 确认 → 验证成功后开门
2. **刷卡开门**：将卡片靠近RC522 → 自动识别 → 开门
3. **远程开门**：手机向巴法云主题发送 `on` → 开门

### 管理员操作

1. 刷卡或输入管理员密码 → 进入管理员菜单
2. 使用 `↑/↓` 选择功能，`C` 确认，`ESC` 返回

## ? 依赖库

- STM32 HAL 库
- 标准C库（string.h、stdio.h）

## ?? 注意事项

1. **首次烧录**：系统会自动将默认数据写入Flash，仅首次运行执行
2. **卡片ID**：需使用RC522读卡器获取实际卡片ID并修改代码中的默认值
3. **Wi-Fi配置**：需修改 `ESP.c` 中的 `ssid` 和 `pswd` 为自己路由器的名称和密码
4. **舵机校准**：不同舵机的0°/180°对应脉宽可能不同，可调整 `Servo_SetAngle` 函数中的脉宽范围
