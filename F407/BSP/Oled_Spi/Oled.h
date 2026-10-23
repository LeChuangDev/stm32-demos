#pragma once  //!防止头文件被重复包含

#include <stdint.h>
#include "sys.h"
#include "./Oled_Spi/Oled_data.h"

/*参数宏定义*********************/

/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_6X8				6
#define OLED_8X16				8


/*IsFilled参数数值*/
#define OLED_UNFILLED			0
#define OLED_FILLED				1

/*********************参数宏定义*/
#define Bit_RESET  0

//数字类型定义
#define OLED_DEC_UNSIGNED   0  //十进制无符号整数
#define OLED_DEC_SIGNED     1  //十进制有符号整数
#define OLED_HEX            2  //十六进制
#define OLED_BIN            3  //二进制 
#define OLED_FLOAT          4   //浮点数

/* 针对 STM32H7 优化的引脚控制 *********************/

// 内联函数强制编译器直接嵌入代码
#define FORCE_INLINE __attribute__((always_inline)) static inline



/*函数声明*********************/

/*初始化函数*/
void OLED_Init(void);

/*更新函数*/
void OLED_Update(void);
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/*显存控制函数*/
void OLED_Clear(void);
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void OLED_Reverse(void);
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void OLED_SetContrast(uint8_t contrast);

/*显示函数*/
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void OLED_ShowNumber(int16_t X, int16_t Y, double Number, uint8_t NumType, uint8_t Length, uint8_t FontSize);
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
int OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, const char *format, ...);
void OLED_ShowText(int16_t x, int16_t y, const char *text, uint8_t fontSize);

