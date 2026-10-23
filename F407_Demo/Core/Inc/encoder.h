#ifndef __ENCODER_H
#define __ENCODER_H

#include "my_code.h"

#define TIM3_CH1  GPIO_Pin_6
#define TIM3_CH4  GPIO_Pin_9

void encoder1_init(void);
void encoder2_init(void);
int Read_Encoder(int TIMX);
    
#endif
