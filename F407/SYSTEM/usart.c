#include "my_code.h"
//!如果使用ucos,则包括下面的头文件即可.
//?如果使用FreeRTOS,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用
#endif


#if 1
__ASM (".global __use_no_semihosting");              

/* FILE is typedef’ d in stdio.h. */
FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//?重定义fputc函数
int fputc(int ch, FILE *f)
{
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
	USART1->DR = (u8) ch;
	return ch;
}
#endif

void Usart_Init(u32 plk,u32 bound)
{
    float temp;
    u16 mantissa;
	u16 fraction;
	temp=(float)(plk*1000000)/(bound*16);//得到USARTDIV@OVER8=0
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分@OVER8=0
    mantissa<<=4;
	mantissa+=fraction;
	RCC->AHB1ENR|=1<<0;
	RCC->APB2ENR|=1<<4;  	 //使能串口1时钟
	GPIO_PZ(GPIOA,RCC_A,PIN9|PIN10,GPIO_Mode_AF,GPIO_Otype_PP,GPIO_Speed_50M,GPIO_Pupd_PU);//PA9,PA10,复用功能,上拉输出
	GPIO_AF(GPIOA,PIN9|PIN10,7);	//PA9和PA10,AF7
	//波特率设置
 	USART1->BRR=mantissa; 	//波特率设置
	USART1->CR1&=~(1<<15); 	//设置OVER8=0，十六倍采样
	USART1->CR1|=1<<3;  	//串口发送使能
#if En_Rx		  	//如果使能了接收
	//使能接收中断
	USART1->CR1|=1<<2;  	//串口接收使能
	USART1->CR1|=1<<5;    	//接收缓冲区非空中断使能
	MY_NVIC_Init(3,3,USART1_IRQn,2);//组2，最低优先级
#endif
	USART1->CR1|=1<<13;  	//串口使能
}

#if En_Rx      //todo:为了避免在接收时出现数据丢失,这里使用中断方式进行数据的接收
u8  Usart_Data[Usart_Len];//?最大的接收字节
u16 Usart_Sta;//*接收状态标记位

//!串口中断服务程序
//todo:简单解释一下Usart_Sta的作用，主要是判断是否接收到数据，接收是否完成
void USART1_IRQHandler(void)
{
    u8 data;//*定义一个数据变量，暂时存放接收到的数据
    if(USART1->SR&1<<5)//!接收缓冲区非空中断
    {
        data=USART1->DR;//?读取数据寄存器
        if((Usart_Sta&0x8000)==0)//*判断是否接收完成
        {
          if(Usart_Sta&0x4000)//*判断是否是接收到OXOD
          {
            if(data!=0x0a)Usart_Sta=0;//接收错误,重新开始
		    else Usart_Sta|=0x8000;	//接收完成了
          }
          else
          {
            if(data==0x0d)Usart_Sta|=0x4000;
				else
				{
				  Usart_Data[Usart_Sta&0X3FFF]=data;
				  Usart_Sta++;
				  if(Usart_Sta>(Usart_Len-1))Usart_Sta=0;//接收数据错误,重新开始接收
				}
          }
        }
    }
}
#endif
