#include "sys.h"
#include "./Oled_Spi/Oled.h"

int main(void){
	Sys_Clock_Init(160,5,2,4);
	//MY_NVIC_PriorityGroupConfig(2);
	OLED_Init();
	OLED_ShowText(0,0,"0XIF82",OLED_8X16);
	OLED_Update();
	while(1){
		
	}
}