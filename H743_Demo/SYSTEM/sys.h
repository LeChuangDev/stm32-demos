#pragma once  //!防止头文件被重复包含

#ifndef __SYS_H
#define __SYS_H

#include "stdio.h"
#include "stdint.h"
#include "stm32h7xx.h"

//*定义一些常用的数据类型短关键字
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef volatile uint32_t  vu32;
typedef volatile uint16_t vu16;
typedef volatile uint8_t  vu8;

// 其他校验宏
#define IS_GPIO_MODE(MODE) \
    (((MODE) == GPIO_MODE_IN) || \
     ((MODE) == GPIO_MODE_OUT) || \
     ((MODE) == GPIO_MODE_AF) || \
     ((MODE) == GPIO_MODE_AIN))

#define IS_GPIO_OTYPE(OTYPE) \
    (((OTYPE) == GPIO_OTYPE_PP) || \
     ((OTYPE) == GPIO_OTYPE_OD))

#define IS_GPIO_SPEED(SPEED) \
    (((SPEED) == GPIO_SPEED_LOW) || \
     ((SPEED) == GPIO_SPEED_MID) || \
     ((SPEED) == GPIO_SPEED_FAST) || \
     ((SPEED) == GPIO_SPEED_HIGH))

#define IS_GPIO_PULL(PULL) \
    (((PULL) == GPIO_PUPD_NONE) || \
     ((PULL) == GPIO_PUPD_PU) || \
     ((PULL) == GPIO_PUPD_PD) || \
     ((PULL) == GPIO_PUPD_RES))

#define IS_GPIO_PIN(PIN) ((PIN) && ((PIN) & 0xFFFF0000U) == 0U)

//////////////////////////////////////////////////////////////////////////////////
//Ex_NVIC_Config专用定义
#define GPIO_A 				0
#define GPIO_B 				1
#define GPIO_C				2
#define GPIO_D 				3
#define GPIO_E 				4
#define GPIO_F 				5
#define GPIO_G 				6
#define GPIO_H 				7
#define GPIO_I 				8
#define GPIO_J 				9
#define GPIO_K 				10

#define FTIR   				1  		//下降沿触发
#define RTIR   				2  		//上升沿触发

//!该宏定义用于GPIO口配置
//todo:GPIO定义
#define GPIO_A 				0
#define GPIO_B 				1
#define GPIO_C				2
#define GPIO_D 				3
#define GPIO_E 				4
#define GPIO_F 				5
#define GPIO_G 				6
#define GPIO_H 				7
#define GPIO_I 				8
#define GPIO_J 				9
#define GPIO_K 				10

//todo:GPIO口时钟定义
#define  RCC_A              1<<0
#define  RCC_B              1<<1
#define  RCC_C              1<<2
#define  RCC_D              1<<3
#define  RCC_E              1<<4
#define  RCC_F              1<<5
#define  RCC_G              1<<6
#define  RCC_H              1<<7
#define  RCC_I              1<<8
#define  RCC_J              1<<9
#define  RCC_K              1<<10

/* GPIO模式定义 */
typedef enum {
    GPIO_MODE_IN      = 0,    // 普通输入模式
    GPIO_MODE_OUT     = 1,    // 普通输出模式
    GPIO_MODE_AF      = 2,    // AF功能模式
    GPIO_MODE_AIN     = 3     // 模拟输入模式
} GPIO_ModeTypeDef;

/* GPIO速度定义 */
typedef enum {
    GPIO_SPEED_LOW    = 0,    // GPIO速度(低速,12M)
    GPIO_SPEED_MID    = 1,    // GPIO速度(中速,60M)
    GPIO_SPEED_FAST   = 2,    // GPIO速度(快速,85M)
    GPIO_SPEED_HIGH   = 3     // GPIO速度(高速,100M)
} GPIO_SpeedTypeDef;

/* GPIO上拉下拉定义 */
typedef enum {
    GPIO_PUPD_NONE    = 0,    // 不带上下拉
    GPIO_PUPD_PU      = 1,    // 上拉
    GPIO_PUPD_PD      = 2,    // 下拉
    GPIO_PUPD_RES     = 3     // 保留
} GPIO_PuPdTypeDef;

/* GPIO输出类型定义 */
typedef enum {
    GPIO_OTYPE_PP     = 0,    // 推挽输出
    GPIO_OTYPE_OD     = 1     // 开漏输出
} GPIO_OTypeTypeDef;


/* GPIO引脚位置定义 */
#define GPIO_PIN_0       (1U << 0)   // 引脚0
#define GPIO_PIN_1       (1U << 1)   // 引脚1
#define GPIO_PIN_2       (1U << 2)   // 引脚2
#define GPIO_PIN_3       (1U << 3)   // 引脚3
#define GPIO_PIN_4       (1U << 4)   // 引脚4
#define GPIO_PIN_5       (1U << 5)   // 引脚5
#define GPIO_PIN_6       (1U << 6)   // 引脚6
#define GPIO_PIN_7       (1U << 7)   // 引脚7
#define GPIO_PIN_8       (1U << 8)   // 引脚8
#define GPIO_PIN_9       (1U << 9)   // 引脚9
#define GPIO_PIN_10      (1U << 10)  // 引脚10
#define GPIO_PIN_11      (1U << 11)  // 引脚11
#define GPIO_PIN_12      (1U << 12)  // 引脚12
#define GPIO_PIN_13      (1U << 13)  // 引脚13
#define GPIO_PIN_14      (1U << 14)  // 引脚14
#define GPIO_PIN_15      (1U << 15)  // 引脚15

/* 常用引脚组合定义 */
#define GPIO_PIN_ALL     (0xFFFFU)   // 所有引脚
#define GPIO_PIN_LOW     (0x00FFU)   // 低8位引脚
#define GPIO_PIN_HIGH    (0xFF00U)   // 高8位引脚

//?时钟总线标志位
#define APB1         1
#define APB2         2
#define APB3         3

//!PAB1时钟总线
#define RCC_TIM2       (uint32_t)(1<<0)
#define RCC_TIM3       (uint32_t)(1<<1)
#define RCC_TIM4       (uint32_t)(1<<2)
#define RCC_TIM5       (uint32_t)(1<<3)
#define RCC_TIM6       (uint32_t)(1<<4)
#define RCC_TIM7       (uint32_t)(1<<5)
#define RCC_TIM12      (uint32_t)(1<<6)
#define RCC_TIM13      (uint32_t)(1<<7)
#define RCC_TIM14      (uint32_t)(1<<8)
#define RCC_LPTIM1     (uint32_t)(1<<9)
#define RCC_SPI2       (uint32_t)(1<<14)
#define RCC_SPI3       (uint32_t)(1<<15)
#define RCC_SPDIFRX    (uint32_t)(1<<16)
#define RCC_USART2     (uint32_t)(1<<17)
#define RCC_USART3     (uint32_t)(1<<18)
#define RCC_UART4      (uint32_t)(1<<19)
#define RCC_UART5      (uint32_t)(1<<20)
#define RCC_I2C1       (uint32_t)(1<<21)
#define RCC_I2C2       (uint32_t)(1<<22)
#define RCC_I2C3       (uint32_t)(1<<23)
#define RCC_HDMICEC    (uint32_t)(1<<27)
#define RCC_DAC12      (uint32_t)(1<<29)
#define RCC_UART7      (uint32_t)(1<<30)
#define RCC_UART8      (uint32_t)(1<<31)

//!PAB2时钟总线
#define RCC_TIM1       (uint32_t)(1<<0)
#define RCC_TIM8       (uint32_t)(1<<1)
#define RCC_USART1     (uint32_t)(1<<4)
#define RCC_USART6     (uint32_t)(1<<5)
#define RCC_SPI1       (uint32_t)(1<<12)
#define RCC_SPI4       (uint32_t)(1<<13)
#define RCC_TIM15      (uint32_t)(1<<16)
#define RCC_TIM16      (uint32_t)(1<<17)
#define RCC_TIM17      (uint32_t)(1<<18)
#define RCC_SPI5       (uint32_t)(1<<20)
#define RCC_SAI1       (uint32_t)(1<<22)
#define RCC_SAI2       (uint32_t)(1<<23)
#define RCC_SAI3       (uint32_t)(1<<24)
#define RCC_DFSDM1     (uint32_t)(1<<28)
#define RCC_HRTIM      (uint32_t)(1<<29)

//!PAB4时钟总线
#define RCC_SYSCFG        (uint32_t)(1<<1)
#define RCC_LPUART1       (uint32_t)(1<<3)
#define RCC_SPI6          (uint32_t)(1<<5)
#define RCC_I2C4          (uint32_t)(1<<7)
#define RCC_LPTIM2        (uint32_t)(1<<9)
#define RCC_LPTIM3        (uint32_t)(1<<10)
#define RCC_LPTIM4        (uint32_t)(1<<11)
#define RCC_LPTIM5        (uint32_t)(1<<12)
#define RCC_COMP12        (uint32_t)(1<<14)
#define RCC_VREF          (uint32_t)(1<<15)
#define RCC_RTC           (uint32_t)(1<<16)
#define RCC_SAI4          (uint32_t)(1<<21)

//!:设置板上自带外设LED,KEY宏定义
#define  Red    1
#define  Green  2
#define  YELLOW 3
#define  None   4
#define  LED(n)      RGB_LED(n)
#define  KEY         GPIO_Get(GPIO_C,PIN13)

//!c此为拓展板上所带外设宏定义
#define  LED0(n)     GPIO_Set(GPIOC,PIN0,n)
#define  LED1(n)     GPIO_Set(GPIOC,PIN2,n)
#define  KEY_1       GPIO_Get(GPIOC,PIN1)
#define  KEY_2       GPIO_Get(GPIOC,PIN3)
#define  KEY_3       GPIO_Get(GPIOD,PIN11)
#define  Hongwei     GPIO_Get(GPIOD,PIN11)
#define  Beep(n)     GPIO_Set(GPIOE,PIN8,n)

//?此为扩展外设初始化函数声明
#define Usart_Len           200//*允许最大接收数据长度为200个字节
#define En_Rx               1//*使能接收功能
extern u8  Usart_Data[Usart_Len]; //?接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u16 Usart_Sta;         		//?接收状态标记

//todo:此函数是板载自带的外设初始化函数
void LED_PZ(void);
void KEY_PZ(void);
void RGB_LED(int color);
//todo:时钟基础延时函数和串口1函数
//?delay:
void delay_init(u16 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
//!usart:
void Usart_Init(u32 bound);

//todo:以下函数定义为my_code.c文件中的函数
void GPIO_Init(GPIO_TypeDef* GPIOx, uint32_t PinMask,
               GPIO_ModeTypeDef Mode, GPIO_OTypeTypeDef OutputType, 
               GPIO_SpeedTypeDef Speed, GPIO_PuPdTypeDef Pull);
void GPIO_AF(GPIO_TypeDef* GPIOx, uint32_t PinMask, uint8_t AlternateFunc);
void GPIO_Set(GPIO_TypeDef* GPIOx, uint16_t PIN, uint8_t status);
uint8_t GPIO_Get(GPIO_TypeDef* GPIOx, uint16_t PIN);
void Ex_Config(uint8_t GPIOx, uint8_t BITx, uint8_t TRIM);
void NVIC_PriorityGroupConfig(uint8_t NVIC_Group);
void NVIC_Init(uint8_t NVIC_PreemptionPriority, 
                 uint8_t NVIC_SubPriority, 
                 IRQn_Type X_IRQn, 
                 uint8_t NVIC_Group);
void Standby(void);
void Soft_Reset (void);
void Cache_Enable(void);
uint8_t Sys_Clock_Config(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);
void NVIC_SetVectorTable(uint32_t baseAddr, uint32_t offset);
void Sys_Clock_Init(u32 plln,u32 pllm,u32 pllp,u32 pllq);

/* 系统控制函数声明 */
void WFI_SET(void);
void INTX_DISABLE(void);
void INTX_ENABLE(void);
void MSR_MSP(uint32_t addr);

#endif

