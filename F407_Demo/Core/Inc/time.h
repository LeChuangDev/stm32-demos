#pragma once//!防止头文件被重复包含
#ifndef __TIME_H
#define __TIME_H

#include "sys.h"

void Time_Init(TIM_TypeDef* Timx, u16 arr, u16 psc);
	
#endif