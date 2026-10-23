#pragma once//!防止头文件被重复包含
#ifndef __SYS_H
#define __SYS_H

#include  <stdint.h>
#include  <stdio.h>
#include  "stm32f4xx.h"

//*定义一些常用的数据类型短关键字
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef volatile uint32_t  vu32;
typedef volatile uint16_t vu16;
typedef volatile uint8_t  vu8;
//位带操作,实现51类似的GPIO控制功能，像51是通过sbit来定义PIo口的
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).M4同M3类似,只是寄存器地址变了.
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x40021014
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40021414
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40021814
#define GPIOH_ODR_Addr    (GPIOH_BASE+20) //0x40021C14
#define GPIOI_ODR_Addr    (GPIOI_BASE+20) //0x40022014

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40021010
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40021410
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40021810
#define GPIOH_IDR_Addr    (GPIOH_BASE+16) //0x40021C10
#define GPIOI_IDR_Addr    (GPIOI_BASE+16) //0x40022010

//定时器操作宏定义
#define RCC_A               1<<0
#define RCC_B               1<<1
#define RCC_C               1<<2
#define RCC_D               1<<3
#define RCC_E               1<<4
#define RCC_F               1<<5
#define RCC_G               1<<6
#define RCC_H               1<<7
#define RCC_I               1<<8

//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  //输出
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr,n)  //输入

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  //输出
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr,n)  //输入
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

// 触发模式定义
typedef enum {
    EXTI_TRIGGER_FALLING = 1,  // 下降沿触发
    EXTI_TRIGGER_RISING  = 2,  // 上升沿触发
    EXTI_TRIGGER_BOTH    = 3   // 双边沿触发
} EXTI_TriggerMode;

#define FTIR   				1  		//下降沿触发
#define RTIR   				2  		//上升沿触发

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


//* GPIO模式定义
#define GPIO_Mode_IN    0     // 普通输入模式
#define GPIO_Mode_OUT   1     // 普通输出模式
#define GPIO_Mode_AF    2     // AF功能模式
#define GPIO_Mode_Ain   3     // 模拟输入模式

//* GPIO速度定义
#define GPIO_Speed_2M   0     // GPIO速度2Mhz
#define GPIO_Speed_25M  1     // GPIO速度25Mhz
#define GPIO_Speed_50M  2     // GPIO速度50Mhz
#define GPIO_Speed_100M 3     // GPIO速度100Mhz

//* GPIO上拉下拉定义
#define GPIO_PuPd_NOPULL 0    // 不带上下拉
#define GPIO_PuPd_PU     1    // 上拉
#define GPIO_PuPd_PD     2    // 下拉
#define GPIO_PuPd_RES    3    // 保留

//* GPIO输出类型定义
#define GPIO_Otype_PP    0    // 推挽输出
#define GPIO_Otype_OD    1    // 开漏输出

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

#define APB1         1
#define APB2         2
#define plk1         42
#define plk2         84

//!RCC_APB1定义
#define RCC_TIM2            (uint32_t)(1 << 0)
#define RCC_TIM3            (uint32_t)(1 << 1)
#define RCC_TIM4            (uint32_t)(1 << 2)
#define RCC_TIM5            (uint32_t)(1 << 3)
#define RCC_TIM6            (uint32_t)(1 << 4)
#define RCC_TIM7            (uint32_t)(1 << 5)
#define RCC_TIM12           (uint32_t)(1 << 6)
#define RCC_TIM13           (uint32_t)(1 << 7)
#define RCC_TIM14           (uint32_t)(1 << 8)
#define RCC_WWDG            (uint32_t)(1 << 11)
#define RCC_SPI2            (uint32_t)(1 << 14)
#define RCC_SPI3            (uint32_t)(1 << 15)
#define RCC_USART2          (uint32_t)(1 << 17)
#define RCC_USART3          (uint32_t)(1 << 18)
#define RCC_UART4           (uint32_t)(1 << 19)
#define RCC_UART5           (uint32_t)(1 << 20)
#define RCC_I2C1            (uint32_t)(1 << 21)
#define RCC_I2C2            (uint32_t)(1 << 22)
#define RCC_I2C3            (uint32_t)(1 << 23)
#define RCC_CAN1            (uint32_t)(1 << 25)
#define RCC_CAN2            (uint32_t)(1 << 26)
#define RCC_PWR             (uint32_t)(1 << 28)
#define RCC_DAC             (uint32_t)(1 << 29)
#define RCC_UART7           (uint32_t)(1 << 30)
#define RCC_UART8           (uint32_t)(1 << 31)

//!RCC_APB2定义
#define RCC_TIM1            (uint32_t)(1 << 0)
#define RCC_TIM8            (uint32_t)(1 << 1)
#define RCC_USART1          (uint32_t)(1 << 4)
#define RCC_USART6          (uint32_t)(1 << 5)
#define RCC_ADC             (uint32_t)(1 << 8)
#define RCC_ADC1            (uint32_t)(1 << 8)
#define RCC_ADC2            (uint32_t)(1 << 9)
#define RCC_ADC3            (uint32_t)(1 << 10)
#define RCC_SDIO            (uint32_t)(1 << 11)
#define RCC_SPI1            (uint32_t)(1 << 12)
#define RCC_SPI4            (uint32_t)(1 << 13)
#define RCC_SYSCFG          (uint32_t)(1 << 14)
#define RCC_TIM9            (uint32_t)(1 << 16)
#define RCC_TIM10           (uint32_t)(1 << 17)
#define RCC_TIM11           (uint32_t)(1 << 18)
#define RCC_SPI5            (uint32_t)(1 << 20)
#define RCC_SPI6            (uint32_t)(1 << 21)
#define RCC_SAI1            (uint32_t)(1 << 22)
#define RCC_LTDC            (uint32_t)(1 << 26)

//!此为板上外设预定义
#define LED0                PFout(9)
#define LED1                PFout(10)
#define KEY0                PBin(8)
#define KEY1                PBin(9)

//!基础函数定义
void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
void Usart_Init(u32 plk,u32 bound);


void GPIO_Init(GPIO_TypeDef* GPIOx, uint32_t PIN,
               uint32_t MODE, uint32_t OTYPE, uint32_t OSPEED, uint32_t PUPD);
void GPIO_AF(GPIO_TypeDef* GPIOx, uint32_t PIN, uint8_t AFx);
void GPIO_Set(GPIO_TypeDef* GPIOx, uint16_t PIN, uint8_t status);
uint8_t GPIO_Get(GPIO_TypeDef* GPIOx, uint16_t PIN);
void EXTI_Config(GPIO_TypeDef* gpioPort, uint16_t pin, EXTI_TriggerMode triggerMode);
void NVIC_PriorityGroupConfig(uint8_t priorityGroup);
void NVIC_Init(IRQn_Type irqNumber, uint8_t preemptPriority, uint8_t subPriority);
void Standby(void);
void Soft_Reset (void);
uint8_t Sys_Clock_Config(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);
void NVIC_SetVectorTable(uint32_t baseAddr, uint32_t offset);
void Sys_Clock_Init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);

/* 系统控制函数声明 */
void WFI_SET(void);
void INTX_DISABLE(void);
void INTX_ENABLE(void);
void MSR_MSP(uint32_t addr);


#endif

