#include "sys.h"
int main(void)
{
	Sys_Clock_Init(336,8,2,7);
	GPIO_Init(GPIOB,GPIO_PIN_13|GPIO_PIN_11,GPIO_Mode_OUT,GPIO_Otype_PP,GPIO_Speed_50M,GPIO_PuPd_PU);
	PBout(13) = 0 ;
	PBout(11) = 1 ;
}
