#include "RC522.h"
#include "RC522_Ins.h"
#include "MySPI.h"
#include <string.h>

#define MAXRLEN 18  // 最大接收数据长度

static void RC522_WriteReg(unsigned char addr, unsigned char val);
static unsigned char RC522_ReadReg(unsigned char addr);
static void RC522_RST_HIGH(void);
static void RC522_RST_LOW(void);
static char RC522_ToCard(unsigned char Command, unsigned char *pInData, 
                         unsigned char InLenByte, unsigned char *pOutData, 
                         unsigned int *pOutLenBit);
static void CalulateCRC(unsigned char *pIndata, unsigned char len, 
                        unsigned char *pOutData);
static char RC522_Request(unsigned char req_code, unsigned char *pTagType);
static char RC522_Anticoll(unsigned char *pSnr);


/**
 * 函数名：RC522_WriteReg
 * 功能描述：向RC522寄存器写入一个字节（底层SPI通信）
 * 输入参数：addr - 寄存器地址，val - 要写入的值
 * 返回值：无
 * 注意事项：SPI通信格式：地址<<1 & 0x7E，最低位为0表示写操作
 */
static void RC522_WriteReg(unsigned char addr, unsigned char val)
{
    RC522_SDA_L;                              // 片选拉低，选中RC522
    MySPI_SwapByte((addr << 1) & 0x7E);       // 发送写地址（最低位=0）
    MySPI_SwapByte(val);                      // 发送数据
    RC522_SDA_H;                              // 片选拉高，释放RC522
}

/**
 * 函数名：RC522_ReadReg
 * 功能描述：从RC522寄存器读取一个字节（底层SPI通信）
 * 输入参数：addr - 寄存器地址
 * 返回值：读取到的寄存器值
 * 注意事项：SPI通信格式：地址<<1 & 0x7E | 0x80，最高位为1表示读操作
 */
static unsigned char RC522_ReadReg(unsigned char addr)
{
    unsigned char val;
    
    RC522_SDA_L;                                          // 片选拉低
    MySPI_SwapByte(((addr << 1) & 0x7E) | 0x80);          // 发送读地址
    val = MySPI_SwapByte(0x00);                           // 发送哑数据，读取返回值
    RC522_SDA_H;                                          // 片选拉高
    
    return val;
}

/**
 * 函数名：Read_RC522
 * 功能描述：从RC522寄存器读取一个字节（对外接口）
 * 输入参数：Address - 寄存器地址
 * 返回值：读取到的寄存器值
 */
unsigned char Read_RC522(unsigned char Address)
{
    return RC522_ReadReg(Address);
}

/**
 * 函数名：Write_RC522
 * 功能描述：向RC522寄存器写入一个字节（对外接口）
 * 输入参数：Address - 寄存器地址，value - 要写入的值
 * 返回值：无
 */
void Write_RC522(unsigned char Address, unsigned char value)
{
    RC522_WriteReg(Address, value);
}


/**
 * 函数名：RC522_RST_HIGH
 * 功能描述：将复位引脚拉高（释放复位）
 * 输入参数：无
 * 返回值：无
 */
static void RC522_RST_HIGH(void)
{
    HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_SET);
}

/**
 * 函数名：RC522_RST_LOW
 * 功能描述：将复位引脚拉低（进入复位状态）
 * 输入参数：无
 * 返回值：无
 */
static void RC522_RST_LOW(void)
{
    HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_RESET);
}


/**
 * 函数名：RC522_Init
 * 功能描述：初始化RC522模块，配置GPIO和寄存器
 * 输入参数：无
 * 返回值：无
 */
void RC522_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 配置PA4(CS)、PA5(SCK)、PA7(MOSI)、PA11(RST)为推挽输出 */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 配置PA6(MISO)为输入上拉 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 硬件复位RC522 */
    RC522_RST_LOW();        // 拉低复位引脚
    HAL_Delay(50);          // 等待50ms
    RC522_RST_HIGH();       // 拉高复位引脚，释放复位
    HAL_Delay(100);         // 等待100ms
    
    /* 软件复位 */
    RC522_Reset();
    HAL_Delay(10);
    
    /* 开启天线并增加驱动功率（提高读卡距离）*/
    Write_RC522(TxControlReg, 0x83);     // 开启天线
    Write_RC522(GsNReg, 0xFF);           // N驱动最大
    Write_RC522(CWGsPReg, 0x3F);         // P驱动最大
    Write_RC522(ModGsPReg, 0x3F);        // 调制驱动最大
    Write_RC522(TxControlReg, 0x83);     // 再次确保天线开启
    HAL_Delay(10);
}

/**
 * 函数名：RC522_Reset
 * 功能描述：软件复位RC522芯片，恢复默认配置
 * 输入参数：无
 * 返回值：MI_OK-成功
 */
char RC522_Reset(void)
{
    Write_RC522(CommandReg, 0x0F);                     // 发送复位命令
    while(Read_RC522(CommandReg) & 0x10);              // 等待复位完成
    
    /* 配置定时器参数 */
    Write_RC522(TModeReg, 0x8D);                       // 定时器模式
    Write_RC522(TPrescalerReg, 0x3E);                  // 预分频值
    Write_RC522(TReloadRegL, 30);                      // 重装载值低位
    Write_RC522(TReloadRegH, 0);                       // 重装载值高位
    Write_RC522(TxAutoReg, 0x40);                      // 发送自动控制
    Write_RC522(ModeReg, 0x3D);                        // 模式寄存器
    
    RC522_AntennaOn();                                 // 开启天线
    
    return MI_OK;
}


/**
 * 函数名：SetBitMask
 * 功能描述：设置寄存器中的指定位（置1）
 * 输入参数：reg - 寄存器地址，mask - 位掩码
 * 返回值：无
 */
void SetBitMask(unsigned char reg, unsigned char mask)
{
    Write_RC522(reg, Read_RC522(reg) | mask);
}

/**
 * 函数名：ClearBitMask
 * 功能描述：清除寄存器中的指定位（置0）
 * 输入参数：reg - 寄存器地址，mask - 位掩码
 * 返回值：无
 */
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    Write_RC522(reg, Read_RC522(reg) & ~mask);
}


/**
 * 函数名：RC522_AntennaOn
 * 功能描述：开启RC522天线（发送射频信号）
 * 输入参数：无
 * 返回值：无
 */
void RC522_AntennaOn(void)
{
    SetBitMask(TxControlReg, 0x03);    // 设置TxControlReg的bit0和bit1
}

/**
 * 函数名：RC522_AntennaOff
 * 功能描述：关闭RC522天线
 * 输入参数：无
 * 返回值：无
 */
void RC522_AntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);  // 清除TxControlReg的bit0和bit1
}


/**
 * 函数名：RC522_ToCard
 * 功能描述：通过RC522与ISO14443卡片通信（发送命令并接收响应）
 * 输入参数：Command - 命令码，pInData - 发送数据，InLenByte - 发送长度
 * 输出参数：pOutData - 接收数据缓冲区，pOutLenBit - 接收数据长度（位）
 * 返回值：MI_OK-成功，MI_NOTAGERR-无卡，MI_ERR-失败
 */
static char RC522_ToCard(unsigned char Command, unsigned char *pInData, 
                         unsigned char InLenByte, unsigned char *pOutData, 
                         unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    
    /* 根据命令设置中断使能 */
    switch (Command)
    {
        case PCD_AUTHENT:      // 验证命令
            irqEn = 0x12;
            waitFor = 0x10;
            break;
        case PCD_TRANSCEIVE:   // 收发命令
            irqEn = 0x77;
            waitFor = 0x30;
            break;
        default:
            break;
    }

    Write_RC522(ComIEnReg, irqEn | 0x80);      // 配置中断
    ClearBitMask(ComIrqReg, 0x80);             // 清除中断标志
    Write_RC522(CommandReg, PCD_IDLE);         // 进入空闲模式
    SetBitMask(FIFOLevelReg, 0x80);            // 清空FIFO
    
    /* 将待发送数据写入FIFO */
    for(i = 0; i < InLenByte; i++)
    {
        Write_RC522(FIFODataReg, pInData[i]);
    }
    
    Write_RC522(CommandReg, Command);          // 执行命令

    /* 收发命令需要设置位帧寄存器 */
    if(Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }

    /* 等待命令完成 */
    i = 1500;
    do
    {
        n = Read_RC522(ComIrqReg);
        i--;
    } while((i != 0) && !(n & 0x01) && !(n & waitFor));
    
    ClearBitMask(BitFramingReg, 0x80);
    
    if(i != 0)
    {
        /* 检查是否有错误 */
        if(!(Read_RC522(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if(n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;          // 无卡错误
            }
            
            /* 接收数据 */
            if(Command == PCD_TRANSCEIVE)
            {
                n = Read_RC522(FIFOLevelReg);   // FIFO中数据字节数
                lastBits = Read_RC522(ControlReg) & 0x07;
                
                if(lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                
                if(n == 0) n = 1;
                if(n > MAXRLEN) n = MAXRLEN;
                
                /* 读取FIFO数据 */
                for(i = 0; i < n; i++)
                {
                    pOutData[i] = Read_RC522(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }
    
    SetBitMask(ControlReg, 0x80);              // 停止当前命令
    Write_RC522(CommandReg, PCD_IDLE);         // 进入空闲模式
    
    return status;
}

/**
 * 函数名：CalulateCRC
 * 功能描述：计算CRC值（用于MIFARE卡片通信）
 * 输入参数：pIndata - 输入数据，len - 数据长度
 * 输出参数：pOutData - CRC结果（2字节）
 * 返回值：无
 */
static void CalulateCRC(unsigned char *pIndata, unsigned char len, 
                        unsigned char *pOutData)
{
    unsigned char i, n;
    
    ClearBitMask(DivIrqReg, 0x04);             // 清除CRC中断标志
    Write_RC522(CommandReg, PCD_IDLE);         // 空闲模式
    SetBitMask(FIFOLevelReg, 0x80);            // 清空FIFO
    
    /* 写入数据到FIFO */
    for(i = 0; i < len; i++)
    {
        Write_RC522(FIFODataReg, *(pIndata + i));
    }
    
    Write_RC522(CommandReg, PCD_CALCCRC);      // 执行CRC计算
    
    /* 等待CRC计算完成 */
    i = 0xFF;
    do
    {
        n = Read_RC522(DivIrqReg);
        i--;
    } while((i != 0) && !(n & 0x04));
    
    /* 读取CRC结果 */
    pOutData[0] = Read_RC522(CRCResultRegL);
    pOutData[1] = Read_RC522(CRCResultRegM);
}


/**
 * 函数名：RC522_Request
 * 功能描述：寻卡（检测天线区域内是否有卡片）
 * 输入参数：req_code - 寻卡指令（0x26或0x52）
 * 输出参数：pTagType - 卡片类型（2字节）
 * 返回值：MI_OK-成功，MI_ERR-失败
 */
static char RC522_Request(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);                    // 清除状态寄存器
    Write_RC522(BitFramingReg, 0x07);                  // 设置位帧
    SetBitMask(TxControlReg, 0x03);                    // 确保天线开启

    ucComMF522Buf[0] = req_code;                       // 发送寻卡命令
    status = RC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
    
    if((status == MI_OK) && (unLen == 0x10))           // 收到16位响应
    {
        *pTagType = ucComMF522Buf[0];                  // 卡片类型高位
        *(pTagType + 1) = ucComMF522Buf[1];            // 卡片类型低位
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**
 * 函数名：RC522_Anticoll
 * 功能描述：防冲突检测，获取卡片唯一序列号（UID）
 * 输入参数：无
 * 输出参数：pSnr - 4字节卡片UID
 * 返回值：MI_OK-成功，MI_ERR-失败
 */
static char RC522_Anticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);                    // 清除状态寄存器
    Write_RC522(BitFramingReg, 0x00);                  // 设置位帧
    ClearBitMask(CollReg, 0x80);                       // 清除冲突寄存器

    ucComMF522Buf[0] = PICC_ANTICOLL1;                 // 防冲突命令
    ucComMF522Buf[1] = 0x20;                           // 默认值

    status = RC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if(status == MI_OK)
    {
        /* 提取4字节UID并计算校验 */
        for(i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];             // 异或校验
        }
        /* 校验第5字节 */
        if(snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80);                         // 恢复冲突寄存器
    return status;
}

/**
 * 函数名：RC522_ReadCardID
 * 功能描述：读取卡片ID（组合寻卡和防冲突）
 * 输入参数：无
 * 输出参数：uid - 4字节卡片UID
 * 返回值：MI_OK-成功，MI_ERR-失败
 */
char RC522_ReadCardID(unsigned char *uid)
{
    char status;
    unsigned char tag_type[2];
    
    status = RC522_Request(PICC_REQIDL, tag_type);     // 先寻卡
    if(status == MI_OK)
    {
        status = RC522_Anticoll(uid);                  // 再防冲突获取UID
    }
    return status;
}
/**
 * 函数名：RC522_Halt
 * 功能描述：让卡片进入休眠状态，以便下次重新寻卡
 * 输入参数：无
 * 返回值：MI_OK-成功
 */
char RC522_Halt(void)
{
    unsigned char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    
    ucComMF522Buf[0] = PICC_HALT;        // 休眠命令
    ucComMF522Buf[1] = 0;
    
    // 计算CRC
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
    
    status = RC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    
    return MI_OK;
}