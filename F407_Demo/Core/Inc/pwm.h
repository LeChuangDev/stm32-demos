#pragma once//!防止头文件被重复包含
#ifndef __PWM_H
#define __PWM_H
#include "sys.h"
#include <stdint.h>

void Pwm_Init(u32 arr,u32 psc);

#endif
