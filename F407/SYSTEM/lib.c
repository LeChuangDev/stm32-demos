#include "OLED.h"
#include "my_code.h"
#include <stm32f4xx.h>
#include "time.h"
#include "Hueidao.h"
#include "K210RX.h"
#include "zigbee.h"

extern int shuzi;
extern int abc;

void LED_RGB(int RGB)
{
	switch (RGB)
	{
		case Green: 
		{GPIOE->BSRRH =PIN14;
			break;}
		case Yellow: 
		{GPIOE->BSRRH =PIN10;
			break;}
		case Red: 
		{GPIOE->BSRRH =PIN15;
			break;}
		default:break;//GPIOE->BSRRL =(PIN8|PIN10|PIN12);
	}
}

void LED_PZ(void)
{
	RCC->AHB1ENR|=1<<4;

	GPIOE->MODER&=~((3<<30)|(3<<20)|(3<<28));
  GPIOE->MODER|=((1<<30)|(1<<20)|(1<<28));

	GPIOE->OTYPER&=~((1<<14)|(1<<10)|(1<<15));
	GPIOE->OTYPER|=((0<<14)|(0<<10)|(1<<15));

	GPIOE->OSPEEDR&=~((3<<30)|(3<<20)|(3<<28));
	GPIOE->OSPEEDR|=((3<<18)|(3<<20)|(3<<28));

	GPIOE->PUPDR&=~((3<<30)|(3<<20)|(3<<28));
	GPIOE->PUPDR|=((1<<30)|(1<<20)|(1<<28));
}

void KEY_PZ(void)
{
	RCC->AHB1ENR|=1<<1;

	GPIOB->MODER|=(3<<16)|(3<<18);
	GPIOB->MODER|=(1<<16)|(1<<18);

	GPIOB->PUPDR|=(3<<16)|(3<<18);
	GPIOB->PUPDR|=(1<<16)|(1<<18);
}


void KEY_Init(void)
{
	//GPIO_PZ(GPIOE,RCC_E,PIN15,GPIO_Mode_IN,GPIO_Otype_PP,GPIO_Speed_100M,GPIO_Pupd_PU);
	RCC->AHB1ENR|=(1<<2)|(1<<5)|(1<<6);
	GPIOC->MODER|=(0<<0)|(0<<8);
	GPIOF->MODER|=0<<28;
	GPIOG->MODER|=0<<0;
	GPIOC->PUPDR|=(1<<0)|(1<<8);
	GPIOF->PUPDR|=1<<28;
	GPIOG->MODER|=1<<0;
}

void Hedwei(void)
{
	RCC->AHB1ENR |=1<<1;

	GPIOB->MODER&=~(3<<8);
	GPIOB->MODER|=0<<8;

	GPIOB->PUPDR&=~(3<<8);
	GPIOB->PUPDR|=1<<8;
}

void BEEP_PZ(void)
{
	RCC->AHB1ENR |=1<<2;

	GPIOC->MODER&=~(3<<2);
	GPIOC->MODER|=1<<2;

	GPIOC->OTYPER&=~(1<<1);
	GPIOC->OTYPER|=0<<1;

	GPIOC->OSPEEDR&=~(3<<2);
	GPIOC->OSPEEDR|=3<<2;

	GPIOC->PUPDR&=~(3<<2);
	GPIOC->PUPDR|=1<<2;
}

//!检测药品装载
inline uint8_t Sb_Hd(void)
{
    static uint8_t cshu = 0; // 静态变量，记录状态

    // 第一次读取 Hongwei 的值
    uint8_t hongwei_value1 = Hongwei;
    delay_ms(10); // 延时 10ms

    // 第二次读取 Hongwei 的值
    uint8_t hongwei_value2 = Hongwei;
    delay_ms(10); // 延时 10ms

    // 如果两次读取的值不一致，重新读取
    if (hongwei_value1 != hongwei_value2) {
        hongwei_value1 = Hongwei;
        delay_ms(10); // 延时 10ms
        hongwei_value2 = Hongwei;
    }

    // 取两次读取的值作为最终值（假设两次读取的值一致）
    uint8_t hongwei_value = hongwei_value1;

    if (hongwei_value == 0) {
        cshu = 1; // 设置状态为 1
        return 1;  // 返回 1
    }

    if (hongwei_value == 1) {
        // 验证 cshu 的值
        if (cshu == 1) {
            return 2; // 返回 2
        }
    }
    return 0; // 默认返回 0
}

void OLED_XS(void)
{
    char *message = NULL; // 用于存储显示的消息
    uint8_t status = Sb_Hd();   // 只调用一次 Sb_Hd()，避免重复调用

    // 根据状态设置显示消息
    switch (status) {
        case 1:
            TIM2->CR1 |= 0x01; // 启动 TIM2
				    szsb();
            message = "药品已装载";
            break;
        case 2:
            message = "药品已卸载";
            break;
        default:
            message = "药品未上车";
            break;
    }

    // 显示消息
    OLED_ShowChinese(2, 4, message);
	// 更新 OLED 显示
	// 显示其他数据
    OLED_ShowNum(90, 0, shuzi, 1, OLED_8X16);
    OLED_ShowNum(0, 20, Shuzi.shu_l, 1, OLED_8X16);
    OLED_ShowNum(30, 20, Shuzi.shu_r, 1, OLED_8X16);
    OLED_ShowNum(50, 20, zhuannwan.Data_l, 1, OLED_8X16);
    OLED_ShowNum(70, 20, zhuannwan.Data_r, 1, OLED_8X16);
    OLED_ShowNum(0, 40, my_Rx.c1, 3, OLED_8X16);
    OLED_ShowNum(30, 40, my_Rx.c2, 1, OLED_8X16);
		OLED_ShowNum(35, 40, my_Rx.c4, 1, OLED_6X8);
    OLED_ShowNum(50, 40, abc, 1, OLED_8X16);
		OLED_ShowNum(120, 32,my_Kx.k2,1,OLED_6X8);
		OLED_ShowNum(100, 32,Usart2_Data[0],1,OLED_6X8);
		OLED_ShowNum(70, 40,Shuzi.Colag,1,OLED_8X16);
		OLED_ShowNum(90, 50,my_Zx.z1,1,OLED_6X8);
		OLED_ShowNum(97, 50,my_Zx.z2,1,OLED_6X8);
		OLED_ShowNum(104, 50,my_Zx.z3,1,OLED_6X8);
		OLED_ShowNum(111, 50,my_Zx.z4,1,OLED_6X8);

    // 更新 OLED 显示
    OLED_Update();
}

