#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "sys.h"
#include "./Oled_Spi/Oled.h"

/*全局变量*********************/

#define OLED_TYPE_SSD1306
#define OLED_CHARSET_UTF8  


// 使用位掩码替代条件分支
FORCE_INLINE void OLED_W_D0(int BitVal)
{
    GPIOA->BSRR = BitVal ? GPIO_PIN_5 : 0;
    GPIOA->BSRR = BitVal ? 0 : (GPIO_PIN_5<<16);
}

FORCE_INLINE void OLED_W_D1(int BitVal)
{
    GPIOA->BSRR = BitVal ? GPIO_PIN_7 : 0;
    GPIOA->BSRR = BitVal ? 0 : (GPIO_PIN_7<<16);
}
/*
FORCE_INLINE void OLED_W_RES(int BitVal)
{
    GPIOC->BSRRL = BitVal ? PIN4 : 0;
    GPIOC->BSRRH = BitVal ? 0 : PIN4;
}*/

FORCE_INLINE void OLED_W_DC(int BitVal)
{
    GPIOC->BSRR = BitVal ? GPIO_PIN_5 : 0;
    GPIOC->BSRR = BitVal ? 0 : (GPIO_PIN_5<<16);
}

FORCE_INLINE void OLED_W_CS(int BitVal)
{
    GPIOA->BSRR = BitVal ? GPIO_PIN_4 : 0;
    GPIOA->BSRR = BitVal ? 0 : (GPIO_PIN_4<<16);
}


/*引脚配置*********************/
/**
 * @file    oled_gpio.h
 * @brief   OLED显示屏GPIO配置宏定义
 * @note    该文件定义了OLED驱动所需的引脚映射和寄存器操作宏
 */

/* 引脚映射宏 - 方便修改引脚定义 */
// 注意：这些宏应与具体的物理引脚对应
#define CS_PIN      GPIO_PIN_4   ///< 片选信号引脚 (GPIOA4)
#define CLK_PIN     GPIO_PIN_5   ///< SPI时钟引脚 (GPIOA5)
#define MOSI_PIN    GPIO_PIN_7   ///< SPI主输出引脚 (GPIOA7)
#define RES_PIN     GPIO_PIN_4   ///< 复位信号引脚 (GPIOC4)
#define DC_PIN      GPIO_PIN_5   ///< 数据/命令选择引脚 (GPIOC5)

/**
 * @brief 设置GPIO模式寄存器 (MODER)
 * @param port GPIO端口 (如GPIOA, GPIOC)
 * @param pin 引脚号 (0-15)
 * @param mode 模式值:
 *             00 = 输入模式
 *             01 = 输出模式
 *             10 = 复用功能模式
 *             11 = 模拟模式
 * @note 该宏执行原子操作，不影响同一端口的其他引脚
 */
#define SET_MODER(port, pin, mode) \
    port->MODER = (port->MODER & ~(3U << ((pin)*2))) | ((mode) << ((pin)*2))

/**
 * @brief 设置GPIO输出速度寄存器 (OSPEEDR)
 * @param port GPIO端口 (如GPIOA, GPIOC)
 * @param pin 引脚号 (0-15)
 * @param speed 速度值:
 *              00 = 低速 (2 MHz)
 *              01 = 中速 (25-50 MHz)
 *              10 = 高速 (50-100 MHz)
 *              11 = 超高速 (100-200 MHz)
 * @note 高速模式可减少信号失真，推荐SPI引脚使用超高速
 */
#define SET_OSPEEDR(port, pin, speed) \
    port->OSPEEDR = (port->OSPEEDR & ~(3U << ((pin)*2))) | ((speed) << ((pin)*2))

/**
 * @brief 设置GPIO上拉/下拉寄存器 (PUPDR)
 * @param port GPIO端口 (如GPIOA, GPIOC)
 * @param pin 引脚号 (0-15)
 * @param pull 上拉/下拉配置:
 *             00 = 无上拉/下拉
 *             01 = 上拉
 *             10 = 下拉
 *             11 = 保留
 * @note 上拉电阻可防止浮空输入，确保信号稳定
 */
#define SET_PUPDR(port, pin, pull) \
    port->PUPDR = (port->PUPDR & ~(3U << ((pin)*2))) | ((pull) << ((pin)*2))


/**
  * @brief  OLED GPIO初始化函数
  * @note   初始化OLED显示屏所需的GPIO引脚配置
  *         包含SPI接口引脚(CS, CLK, MOSI)和控制引脚(RES, DC)
  */
FORCE_INLINE void OLED_GPIO_Init(void)
{
    // 精确延时100ms（假设400MHz时钟）
    // 等待电源稳定，确保OLED上电完成
    volatile uint32_t delay = 40000000UL;
    while(delay--);

    // 使能GPIOA和GPIOC时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    __DSB(); // 数据同步屏障，确保时钟使能完成

    /* 
     * 配置SPI接口引脚 (GPIOA)
     * PA4 -> CS (片选)
     * PA5 -> CLK (时钟)
     * PA7 -> MOSI (主出从入)
     */
    
    // 设置引脚为输出模式 (01)
    SET_MODER(GPIOA, 4, 1);    // CS
    SET_MODER(GPIOA, 5, 1);    // CLK
    SET_MODER(GPIOA, 7, 1);    // MOSI
    
    // 设置引脚速度为最高 (11 = 非常高速)
    SET_OSPEEDR(GPIOA, 4, 3);
    SET_OSPEEDR(GPIOA, 5, 3);
    SET_OSPEEDR(GPIOA, 7, 3);
    
    // 设置引脚为上拉模式 (01)
    SET_PUPDR(GPIOA, 4, 1);    // CS
    SET_PUPDR(GPIOA, 5, 1);    // CLK
    SET_PUPDR(GPIOA, 7, 1);    // MOSI

    /* 
     * 配置控制引脚 (GPIOC)
     * PC4 -> RES (复位)
     * PC5 -> DC (数据/命令选择)
     */
    
    // 设置引脚为输出模式 (01)
    SET_MODER(GPIOC, 4, 1);    // RES
    SET_MODER(GPIOC, 5, 1);    // DC
    
    // 设置引脚速度为最高 (11 = 非常高速)
    SET_OSPEEDR(GPIOC, 4, 3);
    SET_OSPEEDR(GPIOC, 5, 3);
    
    // 设置引脚为上拉模式 (01)
    SET_PUPDR(GPIOC, 4, 1);    // RES
    SET_PUPDR(GPIOC, 5, 1);    // DC

    /* 
     * 设置默认状态:
     * CS高电平 -> 初始不选中OLED
     * CLK高电平 -> SPI时钟空闲状态
     * MOSI低电平 -> 数据线初始低
     * RES高电平 -> 不复位OLED
     * DC高电平 -> 初始数据模式
     */
    GPIOA->BSRR = CS_PIN | CLK_PIN; // 置位CS和CLK (输出高)
    GPIOA->BSRR = (MOSI_PIN<<16);         // 清除MOSI (输出低)
    GPIOC->BSRR = RES_PIN | DC_PIN; // 置位RES和DC (输出高)

    __DSB(); // 数据同步屏障，确保所有配置完成
}

/*********************引脚配置*/


/*通信协议*********************/

/**
  * @brief  SPI高速发送一个字节
  * @param  Byte 要发送的数据 (0x00~0xFF)
  * @note   完全使用内联GPIO函数
  *         循环展开优化性能
  *         最小化指令周期
  */
FORCE_INLINE void OLED_SPI_SendByte(uint8_t Byte)
{
    // 最高位优先(MSB)发送
    OLED_W_D1(!!(Byte & 0x80)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x40)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x20)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x10)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x08)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x04)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x02)); OLED_W_D0(1); OLED_W_D0(0);
    OLED_W_D1(!!(Byte & 0x01)); OLED_W_D0(1); OLED_W_D0(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令值 (0x00~0xFF)
  * @note   组合控制信号操作
  *         最小化状态切换
  */
FORCE_INLINE void OLED_WriteCommand(uint8_t Command)
{
    OLED_W_CS(0);  // 片选使能
    OLED_W_DC(0);  // 命令模式
    OLED_SPI_SendByte(Command);
    OLED_W_CS(1);  // 片选禁用
    OLED_W_DC(1);  // 恢复数据模式(可选)
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入数据的起始地址
  * @param  Count 要写入数据的数量
  * @note   优化数据块传输
  *         减少控制信号切换
  */
FORCE_INLINE void OLED_WriteData(uint8_t *Data, uint16_t Count)
{
    // 检查输入参数的有效性
    if (Data == NULL || Count == 0) {
        // 处理错误：记录日志或返回错误码
        return;
    }
    
    OLED_W_CS(0);  // 片选使能
    OLED_W_DC(1);  // 数据模式
    
    // 高效数据发送循环
    while(Count--) {
        OLED_SPI_SendByte(*Data++);
        // 可选：检查SPI发送状态，增加错误处理
        // if (SPI_SendError()) { ... }
    }
    
    OLED_W_CS(1);  // 片选禁用
}

/*********************通信协议*/

/*硬件配置*********************/

/**
  * @brief  OLED初始化函数
  * @param  无
  * @retval 无
  * @note   使用前必须调用此初始化函数
  *         集成了硬件复位和批量命令发送
  */
void OLED_Init(void)
{
    
    
    // 1. 发送初始化命令序列
    static const uint8_t init_cmds[] = 
	{
        0xAE,       // 关闭显示
        0x20, 0x00, // 设置水平寻址模式 (关键!)
        0xD5, 0x80, // 设置时钟分频比
        0xA8, 0x3F, // 设置多路复用率
        0xD3, 0x00, // 设置显示偏移
        0x40,       // 设置显示开始行
        0xA1,       // 段重映射(正常)
        0xC8,       // COM扫描方向(正常)
        0xDA, 0x12, // COM引脚配置
        0x81, 0xCF, // 设置对比度
        0xD9, 0xF1, // 设置预充电周期
        0xDB, 0x30, // 设置VCOMH
        0xA4,       // 显示内容跟随RAM
        0xA6,       // 正常显示
        0x8D, 0x14, // 电荷泵使能 (关键!)
        0xAF        // 开启显示
    };

    // 2. 初始化GPIO
    OLED_GPIO_Init();

    // 3. 寄存器级命令发送
    for(uint8_t i = 0; i < sizeof(init_cmds); i++) 
    {
        OLED_WriteCommand(init_cmds[i]);
    }
    OLED_Clear();				//清空显存数组
	OLED_Update();				//更新显示，清屏，防止初始化后未显示内容时花屏
}

#define OLED_COLUMN_OFFSET 2
/**
  * @brief  设置OLED显示光标位置
  * @param  Page 指定光标所在的页 (0-7)
  * @param  X 指定光标所在的X轴坐标 (0-127)
  * @retval 无
  * @note   支持多种OLED控制器(SSD1306/SH1106)
  *         自动处理不同屏幕的列偏移
  *         带参数范围检查
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    // 参数有效性检查
    if(Page > 7) Page = 7;  // 页范围限制
    if(X > 127) X = 127;    // X坐标范围限制
    
    /* 自动检测屏幕类型并应用偏移 */
#if defined(OLED_TYPE_SH1106)
    // SH1106屏幕偏移处理 (1.3寸屏)
    X += OLED_COLUMN_OFFSET;  // 默认为2
#elif defined(OLED_TYPE_SSD1306)
    // SSD1306无需偏移 (0.96寸屏)
#else
    // 自动检测逻辑
    static uint8_t oled_type_detected = 0;
    if(!oled_type_detected) {
        // 首次运行时检测屏幕类型
        OLED_WriteCommand(0xAE); // 关闭显示
        
        // 测试偏移写入
        OLED_WriteCommand(0xB0); // 页0
        OLED_WriteCommand(0x10 | (2 >> 4)); // 列高4位
        OLED_WriteCommand(0x00 | (2 & 0x0F)); // 列低4位
        OLED_WriteData(0xFF);   // 测试点
        
        // 检查是否可见
        if(OLED_ReadPixel(0, 0)) {
            oled_type_detected = OLED_TYPE_SSD1306;
        } else {
            oled_type_detected = OLED_TYPE_SH1106;
        }
        
        OLED_Clear();
        OLED_WriteCommand(0xAF); // 重新开启显示
    }
    
    if(oled_type_detected == OLED_TYPE_SH1106) {
        X += OLED_COLUMN_OFFSET;
    }
#endif

    /* 通用设置命令 */
    OLED_WriteCommand(0xB0 | Page);         // 设置页地址 (0xB0-0xB7)
    OLED_WriteCommand(0x10 | (X >> 4));     // 设置列地址高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));   // 设置列地址低4位
}

/*********************硬件配置*/

/*功能函数*********************/


/**
  * OLED显存数组
  * 所有的显示函数，都只是对此显存数组进行读写
  * 随后调用OLED_Update函数或OLED_UpdateArea函数
  * 才会将显存数组的数据发送到OLED硬件，进行显示
  */
uint8_t OLED_DisplayBuf[8][128];

/*功能函数*********************/

/**
  * 函    数：将OLED显存数组更新到OLED屏幕
  * 参    数：无
  * 返 回 值：无
  */
void OLED_Update(void)
{
    /* 直接调用区域更新函数更新整个屏幕 */
    OLED_UpdateArea(0, 0, 128, 64);
}

/**
  * 函    数：将OLED显存数组部分更新到OLED屏幕
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 优化说明：1. 添加边界检查 2. 简化页计算 3. 减少无效更新
  */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    /* 如果宽度或高度为0，直接返回 */
    if (Width == 0 || Height == 0) return;
    
    /* 计算实际屏幕有效区域 */
    int16_t X_start = (X < 0) ? 0 : X;
    int16_t X_end = (X + Width > 128) ? 128 : (X + Width);
    int16_t Y_start = (Y < 0) ? 0 : Y;
    int16_t Y_end = (Y + Height > 64) ? 64 : (Y + Height);
    
    /* 检查是否在屏幕外 */
    if (X_start >= X_end || Y_start >= Y_end) return;
    
    /* 计算起始页和结束页（页范围：0-7） */
    uint8_t start_page = Y_start / 8;
    uint8_t end_page = (Y_end + 7) / 8;  /* 等价于向上取整 */
    
    /* 约束页范围在0-7之间 */
    if (start_page > 7) start_page = 7;
    if (end_page > 8) end_page = 8;  /* 结束页是开区间 */
    
    /* 遍历需要更新的页 */
    for (uint8_t page = start_page; page < end_page; page++) {
        /* 设置光标位置：页地址 + 列起始地址 */
        OLED_SetCursor(page, X_start);
        /* 写入该页中指定列范围的数据 */
        OLED_WriteData(&OLED_DisplayBuf[page][X_start], X_end - X_start);
    }
}

/**
  * 函    数：将OLED显存数组全部清零
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Clear(void)
{
    /* 利用连续内存特性快速清零 */
    memset(OLED_DisplayBuf, 0, sizeof(OLED_DisplayBuf));
}

/**
  * 函    数：将OLED显存数组部分清零
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    /* 处理无效参数 */
    if (Width == 0 || Height == 0) return;
    
    /* 计算有效区域边界 */
    int16_t startX = (X < 0) ? 0 : X;
    int16_t endX = (X + Width > 128) ? 128 : X + Width;
    int16_t startY = (Y < 0) ? 0 : Y;
    int16_t endY = (Y + Height > 64) ? 64 : Y + Height;
    
    /* 检查有效区域 */
    if (startX >= endX || startY >= endY) return;
    
    /* 按页处理提高效率 */
    uint8_t startPage = startY / 8;
    uint8_t endPage = (endY + 7) / 8;  // 向上取整
    
    /* 逐页处理 */
    for (uint8_t page = startPage; page < endPage; page++) {
        /* 计算当前页的Y边界 */
        uint8_t pageStartY = (page == startPage) ? (startY % 8) : 0;
        uint8_t pageEndY = (page == endPage - 1) ? (endY % 8) : 8;
        if (pageEndY == 0) pageEndY = 8;  // 处理整除情况
        
        /* 生成位清除掩码 */
        uint8_t clearMask = 0xFF;
        if (pageStartY > 0 || pageEndY < 8) {
            uint8_t keepMask = (0xFF << pageEndY) | (0xFF >> (8 - pageStartY));
            clearMask = ~keepMask;
        }
        
        /* 清除当前页的指定列 */
        for (int16_t col = startX; col < endX; col++) {
            OLED_DisplayBuf[page][col] &= clearMask;
        }
    }
}

/**
  * 函    数：将OLED显存数组全部取反
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */

void OLED_Reverse(void)
{
    /* 按页处理提高缓存效率 */
    for (uint8_t page = 0; page < 8; page++) {
        for (uint16_t col = 0; col < 128; col++) {
            OLED_DisplayBuf[page][col] ^= 0xFF;
        }
    }
}

/**
  * 函    数：将OLED显存数组部分取反
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    /* 处理无效参数 */
    if (Width == 0 || Height == 0) return;
    
    /* 计算有效区域边界 */
    int16_t startX = (X < 0) ? 0 : X;
    int16_t endX = (X + Width > 128) ? 128 : X + Width;
    int16_t startY = (Y < 0) ? 0 : Y;
    int16_t endY = (Y + Height > 64) ? 64 : Y + Height;
    
    /* 检查有效区域 */
    if (startX >= endX || startY >= endY) return;
    
    /* 按页处理提高效率 */
    uint8_t startPage = startY / 8;
    uint8_t endPage = (endY + 7) / 8;  // 向上取整
    
    /* 逐页处理 */
    for (uint8_t page = startPage; page < endPage; page++) {
        /* 计算当前页的Y边界 */
        uint8_t pageStartY = (page == startPage) ? (startY % 8) : 0;
        uint8_t pageEndY = (page == endPage - 1) ? (endY % 8) : 8;
        if (pageEndY == 0) pageEndY = 8;  // 处理整除情况
        
        /* 生成位反转掩码 */
        uint8_t reverseMask = 0;
        if (pageStartY > 0 || pageEndY < 8) {
            reverseMask = (0xFF >> (8 - pageEndY)) & (0xFF << pageStartY);
        } else {
            reverseMask = 0xFF;
        }
        
        /* 反转当前页的指定列 */
        for (int16_t col = startX; col < endX; col++) {
            OLED_DisplayBuf[page][col] ^= reverseMask;
        }
    }
}

/**
  * @brief  设置OLED显示对比度
  * @param  contrast: 对比度值 (0x00~0xFF, 范围0-255)
  * @retval 无
  */
void OLED_SetContrast(uint8_t contrast) 
{
    // 发送设置对比度命令
    OLED_WriteCommand(0x81);  // 对比度设置指令
    
    // 发送对比度值（自动处理超出范围值）
    OLED_WriteCommand(contrast & 0xFF);  // 按位与确保值在0-255范围内
}

/**
  * 函    数：次方函数
  * 参    数：X 底数
  * 参    数：Y 指数
  * 返 回 值：等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//结果默认为1
	while (Y --)			//累乘Y次
	{
		Result *= X;		//每次把X累乘到结果上
	}
	return Result;
}

/**
  * 函    数：OLED显示一个字符
  * 参    数：X 指定字符左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定字符左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Char 指定要显示的字符，范围：ASCII码可见字符
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16		宽8像素，高16像素
  *                 OLED_6X8		宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
	if (FontSize == OLED_8X16)		//字体为宽8像素，高16像素
	{
		/*将ASCII字模库OLED_F8x16的指定数据以8*16的图像格式显示*/
		OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
	}
	else if(FontSize == OLED_6X8)	//字体为宽6像素，高8像素
	{
		/*将ASCII字模库OLED_F6x8的指定数据以6*8的图像格式显示*/
		OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
	}
}
/**
 * 统一文本显示函数（支持ASCII/GB2312/UTF-8）
 * 参数：x, y - 起始坐标
 *       text - 要显示的文本（支持混合字符）
 *       fontSize - 字体大小（OLED_6X8或OLED_8X16）
 * 说明：自动检测字符编码，支持混合显示ASCII和中文
 */
void OLED_ShowText(int16_t x, int16_t y, const char *text, uint8_t fontSize) {
    // 字体参数设置
    uint8_t asciiWidth = 0;
    uint8_t height = 0;
    
    switch(fontSize) {
        case OLED_8X16:
            asciiWidth = 8;
            height = 16;
            break;
        case OLED_6X8:
            asciiWidth = 6;
            height = 8;
            break;
        default:
            return; // 不支持的字体
    }

    uint16_t index = 0;      // 文本索引
    int16_t xOffset = 0;     // X轴偏移量
    char charBuffer[OLED_UTF8_MAX_LEN + 1]; // 字符缓冲区
    
    // 自动检测编码（根据首个字符判断）
    uint8_t isUTF8 = 0;
    if (text[0] != '\0' && (text[0] & 0x80) && (text[0] & 0x40)) {
        isUTF8 = 1; // UTF-8编码（首字节高位为11）
    }

    while (text[index] != '\0') {
        // 清空缓冲区
        memset(charBuffer, 0, sizeof(charBuffer));
        uint8_t charLen = 0;
        
        // 提取当前字符
        if (isUTF8) {
            // UTF-8编码处理
            uint8_t firstByte = text[index];
            
            if ((firstByte & 0x80) == 0x00) {        // 1字节字符
                charLen = 1;
                charBuffer[0] = firstByte;
            }
            else if ((firstByte & 0xE0) == 0xC0) {   // 2字节字符
                charLen = 2;
                if (text[index + 1] == '\0') break;
                charBuffer[0] = firstByte;
                charBuffer[1] = text[index + 1];
            }
            else if ((firstByte & 0xF0) == 0xE0) {   // 3字节字符（常用汉字）
                charLen = 3;
                if (text[index + 1] == '\0' || text[index + 2] == '\0') break;
                charBuffer[0] = firstByte;
                charBuffer[1] = text[index + 1];
                charBuffer[2] = text[index + 2];
            }
            else if ((firstByte & 0xF8) == 0xF0) {   // 4字节字符
                charLen = 4;
                if (text[index + 1] == '\0' || text[index + 2] == '\0' || 
                    text[index + 3] == '\0') break;
                charBuffer[0] = firstByte;
                charBuffer[1] = text[index + 1];
                charBuffer[2] = text[index + 2];
                charBuffer[3] = text[index + 3];
            }
            else {
                index++;  // 跳过无效字节
                continue;
            }
        } else {
            // GB2312编码处理
            if ((text[index] & 0x80) == 0x00) {  // ASCII字符
                charLen = 1;
                charBuffer[0] = text[index];
            } else {                            // 中文字符
                charLen = OLED_CHN_CHAR_WIDTH;
                if (text[index + 1] == '\0') break;
                charBuffer[0] = text[index];
                charBuffer[1] = text[index + 1];
            }
        }
        
        index += charLen;  // 移动到下一个字符
        
        // 处理单字节字符（ASCII）
        if (charLen == 1) {
            uint8_t charCode = (uint8_t)charBuffer[0];
            const uint8_t *charData = NULL;
            
            // 只显示可打印字符
            if (charCode >= ' ' && charCode <= '~') {
                if (fontSize == OLED_8X16) {
                    charData = OLED_F8x16[charCode - ' '];
                } else {
                    charData = OLED_F6x8[charCode - ' '];
                }
            }
            
            if (charData) {
                OLED_ShowImage(x + xOffset, y, asciiWidth, height, charData);
                xOffset += asciiWidth;
            } else {
                // 不可打印字符显示为空格
                xOffset += asciiWidth;
            }
            continue;
        }
        
        // 处理多字节字符（中文/特殊字符）
        uint16_t glyphIndex = 0;
        uint8_t found = 0;
        
        // 在字模库中查找字符 - 使用实际数据结构
        for (glyphIndex = 0; OLED_CF16x16[glyphIndex].Index[0] != '\0'; glyphIndex++) {
            if (strcmp(OLED_CF16x16[glyphIndex].Index, charBuffer) == 0) {
                found = 1;
                break;
            }
        }
        
        // 根据字体处理多字节字符
        if (fontSize == OLED_8X16) {
            if (found) {
                // 显示16x16中文字符 - 使用实际数据结构
                OLED_ShowImage(x + xOffset, y, 16, 16, OLED_CF16x16[glyphIndex].Data);
                xOffset += 16;
            } else {
                // 未找到字模时显示问号
                const uint8_t *charData = OLED_F6x8['?' - ' '];
                if (charData) {
                    OLED_ShowImage(x + xOffset, y, 6, 8, charData);
                }
                xOffset += 6;
            }
        } else {
            // 小字体显示问号
            const uint8_t *charData = OLED_F6x8['?' - ' '];
            if (charData) {
                OLED_ShowImage(x + xOffset, y, 6, 8, charData);
            }
            xOffset += 6;
        }
    }
}
/**
  * 函    数：OLED显示数字（支持多种格式）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Number 指定要显示的数字
  *           - 浮点数：直接传入double类型
  *           - 其他类型：传入对应数字（自动转换）
  * 参    数：NumType 指定数字类型
  *           - OLED_DEC_UNSIGNED：十进制无符号整数
  *           - OLED_DEC_SIGNED：十进制有符号整数
  *           - OLED_HEX：十六进制
  *           - OLED_BIN：二进制
  *           - OLED_FLOAT：浮点数（最多3位小数，自动去除末尾0）
  * 参    数：Length 指定数字长度（根据类型不同含义不同）
  *           - 整数类型：显示的数字位数
  *           - 浮点数：整数部分位数
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16     宽8像素，高16像素
  *                 OLED_6X8      宽6像素，高8像素
  * 返 回 值：无
  * 说    明：调用此函数后，需调用更新函数才能呈现在屏幕上
  */
void OLED_ShowNumber(int16_t X, int16_t Y, double Number, uint8_t NumType, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, digit;
    uint32_t intPart, fracPart;
    double fractional;

    switch (NumType)
    {
        // 十进制无符号整数
        case OLED_DEC_UNSIGNED:
            for (i = 0; i < Length; i++) {
                digit = (uint32_t)Number / OLED_Pow(10, Length - i - 1) % 10;
                OLED_ShowChar(X + i * FontSize, Y, digit + '0', FontSize);
            }
            break;
        
        // 十进制有符号整数
        case OLED_DEC_SIGNED:
            if (Number >= 0) {
                OLED_ShowChar(X, Y, '+', FontSize);
            } else {
                OLED_ShowChar(X, Y, '-', FontSize);
                Number = -Number;
            }
            for (i = 0; i < Length; i++) {
                digit = (uint32_t)Number / OLED_Pow(10, Length - i - 1) % 10;
                OLED_ShowChar(X + (i + 1) * FontSize, Y, digit + '0', FontSize);
            }
            break;
        
        // 十六进制
        case OLED_HEX:
            for (i = 0; i < Length; i++) {
                digit = (uint32_t)Number / OLED_Pow(16, Length - i - 1) % 16;
                if (digit < 10) {
                    OLED_ShowChar(X + i * FontSize, Y, digit + '0', FontSize);
                } else {
                    OLED_ShowChar(X + i * FontSize, Y, digit - 10 + 'A', FontSize);
                }
            }
            break;
        
        // 二进制
        case OLED_BIN:
            for (i = 0; i < Length; i++) {
                digit = (uint32_t)Number / OLED_Pow(2, Length - i - 1) % 2;
                OLED_ShowChar(X + i * FontSize, Y, digit + '0', FontSize);
            }
            break;
        
        // 浮点数（最多3位小数，自动去除末尾0）
        case OLED_FLOAT:
            // 处理符号
            if (Number >= 0) {
                OLED_ShowChar(X, Y, '+', FontSize);
            } else {
                OLED_ShowChar(X, Y, '-', FontSize);
                Number = -Number;
            }
            
            // 提取整数和小数部分
            intPart = (uint32_t)Number;
            fractional = Number - intPart;
            fracPart = round(fractional * 1000);  // 四舍五入到3位小数
            
            // 处理小数进位
            if (fracPart >= 1000) {
                intPart++;
                fracPart -= 1000;
            }
            
            // 显示整数部分
            for (i = 0; i < Length; i++) {
                digit = intPart / OLED_Pow(10, Length - i - 1) % 10;
                OLED_ShowChar(X + (i + 1) * FontSize, Y, digit + '0', FontSize);
            }
            
            // 将小数部分转换为3位数字（百位、十位、个位）
            uint8_t fracDigits[3];
            fracDigits[0] = fracPart / 100;        // 百位（0-9）
            fracDigits[1] = (fracPart / 10) % 10;  // 十位（0-9）
            fracDigits[2] = fracPart % 10;         // 个位（0-9）
            
            // 计算实际需要显示的小数位数（从后往前去除末尾0）
            uint8_t fracLen = 3;
            while (fracLen > 1 && fracDigits[fracLen - 1] == 0) {
                fracLen--;
            }
            
            // 显示小数点和小数部分
            OLED_ShowChar(X + (Length + 1) * FontSize, Y, '.', FontSize);
            for (i = 0; i < fracLen; i++) {
                OLED_ShowChar(X + (Length + 2 + i) * FontSize, Y, fracDigits[i] + '0', FontSize);
            }
            break;
    }
}

/**
  * 函    数：OLED显示图像
  * 参    数：X 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定图像左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Width 指定图像的宽度，范围：0~128
  * 参    数：Height 指定图像的高度，范围：0~64
  * 参    数：Image 指向图像数据的指针（按行存储）
  * 返 回 值：无
  * 说    明：优化了边界处理、坐标计算效率和内存访问方式
  */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    /* 计算图像实际在屏幕上显示的区域 */
    int16_t startX = (X < 0) ? 0 : X;
    int16_t endX = (X + Width > 128) ? 128 : X + Width;
    int16_t startY = (Y < 0) ? 0 : Y;
    int16_t endY = (Y + Height > 64) ? 64 : Y + Height;
    
    /* 如果完全不可见则直接返回 */
    if (endX <= startX || endY <= startY) return;
    
    /* 仅清除实际显示区域 */
    OLED_ClearArea(startX, startY, endX - startX, endY - startY);
    
    /* 计算页偏移量和位偏移量 */
    int16_t startPage = (Y < 0) ? (Y / 8) - 1 : Y / 8;
    int16_t pageOffset = (Y < 0) ? (8 + (Y % 8)) : (Y % 8);
    int16_t visiblePages = ((endY + 7) / 8) - startPage;
    
    /* 遍历图像涉及的相关页 */
    for (int16_t page = 0; page < visiblePages; page++)
    {
        int16_t currentPage = startPage + page;
        
        /* 跳过屏幕外的页 */
        if (currentPage < 0 || currentPage > 7) continue;
        
        /* 遍历图像宽度 */
        for (int16_t col = 0; col < Width; col++)
        {
            int16_t screenCol = X + col;
            
            /* 跳过屏幕外的列 */
            if (screenCol < 0 || screenCol >= 128) continue;
            
            /* 获取当前图像字节 */
            uint8_t imgByte = Image[page * Width + col];
            
            /* 处理当前页 */
            if (pageOffset != 0)
            {
                OLED_DisplayBuf[currentPage][screenCol] |= imgByte << pageOffset;
                
                /* 处理跨页数据（如果有下一页） */
                if (page < visiblePages - 1 && (currentPage + 1) <= 7)
                {
                    OLED_DisplayBuf[currentPage + 1][screenCol] |= imgByte >> (8 - pageOffset);
                }
            }
            else
            {
                /* 无偏移时直接写入 */
                OLED_DisplayBuf[currentPage][screenCol] = imgByte;
            }
        }
    }
}

/**
  * 函    数：OLED使用printf函数打印格式化字符串
  * 参    数：X 指定格式化字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
  * 参    数：Y 指定格式化字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：FontSize 指定字体大小
  *           范围：OLED_8X16     宽8像素，高16像素
  *                 OLED_6X8      宽6像素，高8像素
  * 参    数：format 指定要显示的格式化字符串
  * 参    数：... 格式化字符串参数列表
  * 返 回 值：实际显示的字符数量（负值表示出错）
  * 说    明：1. 调用此函数后需调用更新函数才能呈现在屏幕上
  *          2. 使用安全缓冲区处理防止溢出
  *          3. 自动处理超长字符串显示
  */
int OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, const char *format, ...)
{
    // 根据字体大小确定行缓冲区大小
    const uint8_t LINE_BUF_SIZE = (FontSize == OLED_8X16) ? 16 : 21;
    char line_buf[LINE_BUF_SIZE + 1];  // +1 for null terminator
    //char char_buf[2] = {0};           // 单字符缓冲区
    
    va_list arg;
    va_start(arg, format);
    
    // 使用vsnprintf计算所需缓冲区大小
    int needed = vsnprintf(NULL, 0, format, arg);
    va_end(arg);
    
    if (needed < 0) return -1;  // 格式化错误
    
    // 处理短字符串（不超过一行）
    if (needed <= LINE_BUF_SIZE) {
        char text_buf[needed + 1];
        va_start(arg, format);
        vsnprintf(text_buf, sizeof(text_buf), format, arg);
        va_end(arg);
        OLED_ShowText(X, Y, text_buf, FontSize);
        return needed;
    }
    
    // 处理长字符串（多行显示）
    int chars_displayed = 0;
    int current_line = 0;
    const int line_height = (FontSize == OLED_8X16) ? 16 : 8;
    
    va_start(arg, format);
    while (chars_displayed < needed) {
        // 提取一行内容
        int n = vsnprintf(line_buf, sizeof(line_buf), format, arg);
        if (n <= 0) break;
        
        // 处理格式字符串偏移
        format += n;
        
        // 显示当前行
        OLED_ShowText(X, Y + current_line * line_height, line_buf, FontSize);
        
        chars_displayed += n;
        current_line++;
        
        // 检查屏幕空间是否用完
        if ((Y + (current_line + 1) * line_height) > 63) break;
    }
    va_end(arg);
    
    return chars_displayed;
}


