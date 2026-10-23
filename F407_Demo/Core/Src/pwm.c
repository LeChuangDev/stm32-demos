#include "pwm.h"

/**
  * @brief  PWM初始化函数
  * @param  arr: 自动重装载值，决定PWM周期
  * @param  psc: 预分频系数，用于调整定时器时钟频率
  * @note   配置TIM1的通道1（PE9）和通道2（PE11）输出PWM信号
  * @note   高级定时器时钟说明:
  *          - 时钟源来自APB2总线
  *          - 当APB2分频系数≥2时(PPRE2≥2)，定时器时钟为APB2时钟的2倍
  *          - 在F407中APB2时钟=84MHz，故定时器时钟=168MHz
  *          - 定时器溢出时间计算: Tout = ((arr + 1) * (psc + 1)) / Ft (us)
  *          - Ft = 定时器工作频率(MHz)
  */
void Pwm_Init(u32 arr, u32 psc) {
    // 1. GPIO引脚配置 ---------------------------------
    // 初始化PE9和PE11为复用功能（连接定时器）
    GPIO_Init(GPIOE, GPIO_PIN_9 | GPIO_PIN_11, 
              GPIO_Mode_AF,    // 复用模式
              GPIO_Otype_PP,   // 推挽输出
              GPIO_Speed_100M, // 高速100MHz
              GPIO_PuPd_PU);   // 上拉电阻
    
    // 将PE9和PE11映射到AF1（TIM1的复用功能）
    GPIO_AF(GPIOE, GPIO_PIN_9 | GPIO_PIN_11, 1); 

    // 2. 定时器基础配置 -------------------------------
    do{ RCC->APB2ENR |= 1 << 0;  }while(0); // 使能TIM1时钟（APB2总线）
    TIM1->ARR = arr;         // 设置自动重装载值（决定PWM周期）
    TIM1->PSC = psc;         // 设置预分频器（降低定时器时钟频率）
    
    /* 定时器时钟频率说明：
     *  - APB2时钟 = 84MHz (来自系统时钟配置)
     *  - 当APB2分频系数≥2时，TIM1时钟 = APB2时钟 × 2 = 168MHz
     *  - 实际定时器频率 = 168MHz / (psc + 1)
     *  - PWM周期 = ((arr + 1) × (psc + 1)) / 168 (us)
     * 示例：psc=0, arr=1679 → PWM周期 = 10us (100kHz)
     */

    // 3. PWM模式配置 ---------------------------------
    // 配置通道1（PE9）为PWM模式1（OC1M = 110b）
    TIM1->CCMR1 |= (6 << 4);  // OC1M[2:0] = 110 (PWM模式1)
    TIM1->CCMR1 |= (1 << 3);  // OC1PE = 1 (使能预装载寄存器)
    
    // 配置通道2（PE11）为PWM模式1（OC2M = 110b）
    TIM1->CCMR1 |= (6 << 12); // OC2M[2:0] = 110 (PWM模式1)
    TIM1->CCMR1 |= (1 << 11); // OC2PE = 1 (使能预装载寄存器)

    // 4. 输出控制配置 ---------------------------------
    TIM1->CCER |= (1 << 0);   // CC1E = 1 (使能通道1输出)
    TIM1->CCER &= ~(1 << 1);  // CC1P = 0 (高电平为有效电平)
    TIM1->CCER |= (1 << 4);   // CC2E = 1 (使能通道2输出)
    TIM1->CCER &= ~(1 << 5);  // CC2P = 0 (高电平为有效电平)

    // 5. 高级控制寄存器配置 ---------------------------
    TIM1->CR1 |= 1 << 7;      // ARPE = 1 (ARR启用预装载缓冲)
    TIM1->CR1 |= 1 << 0;      // CEN = 1 (启动定时器计数)
    TIM1->BDTR |= 1 << 15;    // MOE = 1 (主输出使能，关键PWM输出开关)
	
}



