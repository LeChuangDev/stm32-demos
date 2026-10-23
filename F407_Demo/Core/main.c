#include "sys.h"
#include "pwm.h"
#include "./Oled_Spi/Oled.h"
#include "./Jy61_P/Jy61_P.h"
int main(void){
	
	Sys_Clock_Init(336,8,2,7);//168 MHz SYSCLK + 48 MHz USB 时钟
	NVIC_PriorityGroupConfig(2);
	Jy61_Init(115200);
	OLED_Init();
	
	while(1){
		OLED_ShowNumber(0,0,SAngle.Angle[2],OLED_FLOAT,3,OLED_8X16);
		OLED_Update();
		
	}
}
