#ifndef __RC522_INS_H
#define __RC522_INS_H

/*==========================================================================*/
/*                          RC522 寄存器地址定义                             */
/*==========================================================================*/
#define CommandReg       0x01      // 命令寄存器
#define ComIEnReg        0x02      // 中断使能寄存器
#define DivIEnReg        0x03      // 分频器中断使能寄存器
#define ComIrqReg        0x04      // 通信中断请求寄存器
#define DivIrqReg        0x05      // 分频器中断请求寄存器
#define ErrorReg         0x06      // 错误状态寄存器
#define Status1Reg       0x07      // 状态寄存器1
#define Status2Reg       0x08      // 状态寄存器2
#define FIFODataReg      0x09      // FIFO数据缓冲区
#define FIFOLevelReg     0x0A      // FIFO数据长度寄存器
#define WaterLevelReg    0x0B      // FIFO水印寄存器
#define ControlReg       0x0C      // 控制寄存器
#define BitFramingReg    0x0D      // 位帧调节寄存器
#define CollReg          0x0E      // 冲突检测寄存器
#define ModeReg          0x11      // 模式寄存器
#define TxModeReg        0x12      // 发送模式寄存器
#define RxModeReg        0x13      // 接收模式寄存器
#define TxControlReg     0x14      // 发送控制寄存器
#define TxAutoReg        0x15      // 发送自动控制寄存器
#define TxSelReg         0x16      // 发送器选择寄存器
#define RxSelReg         0x17      // 接收器选择寄存器
#define RxThresholdReg   0x18      // 接收器阈值寄存器
#define DemodReg         0x19      // 解调器寄存器
#define MfTxReg          0x1C      // Mifare发送寄存器
#define MfRxReg          0x1D      // Mifare接收寄存器
#define SerialSpeedReg   0x1F      // 串行速度寄存器
#define CRCResultRegM    0x21      // CRC结果寄存器高位
#define CRCResultRegL    0x22      // CRC结果寄存器低位
#define ModWidthReg      0x24      // 调制宽度寄存器
#define RFCfgReg         0x26      // RF配置寄存器
#define GsNReg           0x27      // 选通N寄存器
#define CWGsPReg         0x28      // CW选通P寄存器
#define ModGsPReg        0x29      // 调制选通P寄存器
#define TModeReg         0x2A      // 定时器模式寄存器
#define TPrescalerReg    0x2B      // 定时器预分频寄存器
#define TReloadRegH      0x2C      // 定时器重装载值高位
#define TReloadRegL      0x2D      // 定时器重装载值低位
#define TCounterValRegH  0x2E      // 定时器计数器值高位
#define TCounterValRegL  0x2F      // 定时器计数器值低位
#define TestSel1Reg      0x31      // 测试选择寄存器1
#define TestSel2Reg      0x32      // 测试选择寄存器2
#define TestPinEnReg     0x33      // 测试引脚使能寄存器
#define TestPinValueReg  0x34      // 测试引脚值寄存器
#define TestBusReg       0x35      // 测试总线寄存器
#define AutoTestReg      0x36      // 自动测试寄存器
#define VersionReg       0x37      // 版本寄存器
#define AnalogTestReg    0x38      // 模拟测试寄存器
#define TestDAC1Reg      0x39      // 测试DAC1寄存器
#define TestDAC2Reg      0x3A      // 测试DAC2寄存器
#define TestADCReg       0x3B      // 测试ADC寄存器

/*==========================================================================*/
/*                          RC522 命令集定义                                 */
/*==========================================================================*/
#define PCD_IDLE         0x00      // 取消当前命令，进入空闲模式
#define PCD_AUTHENT      0x0E      // 验证密钥
#define PCD_RECEIVE      0x08      // 接收数据
#define PCD_TRANSMIT     0x04      // 发送数据
#define PCD_TRANSCEIVE   0x0C      // 发送并接收数据
#define PCD_RESETPHASE   0x0F      // 复位相位
#define PCD_CALCCRC      0x03      // 计算CRC

/*==========================================================================*/
/*                          NFC 芯片指令定义                                 */
/*==========================================================================*/
#define NFC_CMD_IDLE                 0x00      // 空闲模式
#define NFC_CMD_CONFIG               0x01      // 配置模式
#define NFC_CMD_GENERATE_RANDOMID    0x02      // 生成随机ID
#define NFC_CMD_CALC_CRC             0x03      // 计算CRC
#define NFC_CMD_TRANSMIT             0x04      // 发送数据
#define NFC_CMD_NO_CMD_CHANGE        0x07      // 命令不变
#define NFC_CMD_RECEIVE              0x08      // 接收数据
#define NFC_CMD_TRANSCEIVE           0x0C      // 发送并接收数据
#define NFC_CMD_AUTO_COLL            0x0D      // 自动防冲突
#define NFC_CMD_SOFT_RESET           0x0F      // 软复位

/*==========================================================================*/
/*                          卡片命令定义                                    */
/*==========================================================================*/
#define PICC_REQIDL      0x26      // 寻卡指令 - 天线区域内未进入休眠状态的卡
#define PICC_REQALL      0x52      // 寻卡指令 - 天线区域内的所有卡
#define PICC_ANTICOLL1   0x93      // 防冲突指令 - Cascade Level 1
#define PICC_ANTICOLL2   0x95      // 防冲突指令 - Cascade Level 2
#define PICC_AUTHENT1A   0x60      // 验证密钥A
#define PICC_AUTHENT1B   0x61      // 验证密钥B
#define PICC_READ        0x30      // 读块数据
#define PICC_WRITE       0xA0      // 写块数据
#define PICC_DECREMENT   0xC0      // 块值递减
#define PICC_INCREMENT   0xC1      // 块值递增
#define PICC_RESTORE     0xC2      // 块值恢复
#define PICC_TRANSFER    0xB0      // 块值传输
#define PICC_HALT        0x50      // 卡片休眠

/*==========================================================================*/
/*                          函数返回值定义                                  */
/*==========================================================================*/
#define MI_OK            0         // 操作成功
#define MI_NOTAGERR      1         // 无卡错误
#define MI_ERR           2         // 操作失败

#endif /* __RC522_INS_H */