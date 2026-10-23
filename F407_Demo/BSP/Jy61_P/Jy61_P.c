#include "./Jy61_P/Jy61_P.h"

struct StcAngle SAngle;
struct StcGyro  SGyro;
struct StcAcc   SAcc;

void Jy61_Init(uint32_t bound){
   float Temp;
   u16 mantissa,fraction;
   Temp=(float)(1000000)*bound/(bound*16);
   mantissa=Temp;
   fraction=(Temp-mantissa)*16;
   mantissa<<=4;
   mantissa+=fraction;
   RCC->APB1ENR|=1<<18;//*使能串口三时钟.
   //?GPIO口复用初始化
   GPIO_Init(GPIOB,GPIO_PIN_11|GPIO_PIN_10,GPIO_Mode_AF,GPIO_Otype_PP,GPIO_Speed_50M,GPIO_PuPd_PU);//*PB11,PB10,复用功能,上拉输出
   GPIO_AF(GPIOB,GPIO_PIN_11|GPIO_PIN_10,7);    //*PB11和PB10,AF7
   //!设置波特率
   USART3->BRR=mantissa;//*写入波特率
   USART3->CR1&=~(1<<15);//*将OVER8位置0，采用十六倍采样
   USART3->CR1|=(1<<3);//*使能串口发送功能.
    //使能接收中断
   USART3->CR1|=1<<2;      //串口接收使能
   USART3->CR1|=1<<5;        //接收缓冲区非空中断使能
   NVIC_Init(USART3_IRQn,0,0);//组2，
   USART3->CR1|=1<<13;      //串口使能
}

//串口中断服务程序
void USART3_IRQHandler(void){
    if(USART3->SR&(1<<5))  //判断中断非空
    {
        uint16_t Data =(USART3->DR & (uint16_t)0x01FF);
        Get_Jy61(Data);
    }
    USART3->SR&=~(1<<5);  //清除中断标志位
}

/***********采集JY61P数据**************/
void Get_Jy61(uint16_t Data){
    static u8 RX_Data[250];
    static u8 len = 0;
    RX_Data[len++]=Data;    //将收到的数据存入缓冲区中
    if (RX_Data[0]!=0x55) //数据头不对，则重新开始寻找0x55数据头
    {
        len=0;
        return;
    }
    if (len<11) return;//数据不满11个，则返回
    else
    {
     switch(RX_Data[1])//判断数据是哪种数据，然后将其拷贝到对应的结构体中，有些数据包需要通过上位机打开对应的输出后，才能接收到这个数据包的数据
     {
        case 0x51:
                        memcpy(&SAcc,&RX_Data[2],8);
                        for(uint8_t j = 0; j < 3; j++)
                            {
                                SAcc.Acc[j] = (float)SAcc.Acc_S[j]/32768*16;
                            }
                        break;
        case 0x52:
                        memcpy(&SGyro,&RX_Data[2],8);
                        for(uint8_t j = 0; j < 3; j++)
                            {
                                SGyro.Gyro[j] = (float)SGyro.Gyro_S[j]/32768*2000;
                            }
                        break;
        case 0x53:
                        memcpy(&SAngle,&RX_Data[2],8);
                        for(uint8_t j = 0; j < 3; j++)
                            {
                               SAngle.Angle[j] = (float)SAngle.Angle_S[j]/32768*180;
                            }
                        break;
     }
     len=0;//清空缓存区
 }
}
