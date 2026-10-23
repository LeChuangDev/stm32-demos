#include "time.h"

/**
  * @brief  通用定时器初始化函数
  * @param  Timx: 定时器指针 (TIM2-TIM5, TIM9-TIM14)
  * @param  arr: 自动重装载值，决定定时周期
  * @param  psc: 预分频系数，用于调整定时器时钟频率
  * @note   定时器时钟源：
  *         - APB1 定时器：TIM2-TIM5, TIM12-TIM14
  *         - APB2 定时器：TIM9-TIM11
  *         当PPRE1/PPRE2分频系数>1时，定时器时钟为APB时钟的2倍
  *         定时器溢出时间：Tout = ((arr + 1) * (psc + 1)) / Ft (秒)
  *         Ft = 定时器工作频率(Hz)
  */
void Time_Init(TIM_TypeDef* Timx, uint16_t arr, uint16_t psc) {
    // 通过地址差判断总线归属 (STM32F4内存映射)
    const uint32_t addr = (uint32_t)Timx;
    const uint32_t apb1_base = APB1PERIPH_BASE;  // 0x40000000
    const uint32_t apb2_base = APB2PERIPH_BASE;  // 0x40010000
    
    // APB1总线定时器使能 (TIM2-TIM5, TIM12-TIM14)
    if (addr >= apb1_base && addr < apb2_base) {
        // 计算定时器索引：偏移量/0x400 (每个定时器占1KB空间)
        uint32_t index = (addr - apb1_base) >> 10;
        // 设置对应使能位：TIM2->bit0, TIM3->bit1, ... TIM14->bit8
        RCC->APB1ENR |= (1UL << (index + RCC_APB1ENR_TIM2EN_Pos));
    }
    // APB2总线定时器使能 (TIM9-TIM11)
    else if (addr >= apb2_base) {
        // TIM9特殊处理：固定偏移0x4000 (16KB)
        if (addr == (apb2_base + 0x4000)) {        // TIM9
            RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
        }
        else if (addr == (apb2_base + 0x4400)) {    // TIM10
            RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
        }
        else if (addr == (apb2_base + 0x4800)) {    // TIM11
            RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
        }
    }

    // 核心寄存器配置
    Timx->ARR = arr;        // 设置自动重装载值
    Timx->PSC = psc;        // 设置预分频器
    Timx->EGR = TIM_EGR_UG; // 产生更新事件，加载预装载值
    
    // 单指令配置使能位（避免读-修改-写操作）
    Timx->CR1 = TIM_CR1_CEN | TIM_CR1_ARPE; // 使能计数器+开启自动重装载预加载
}


