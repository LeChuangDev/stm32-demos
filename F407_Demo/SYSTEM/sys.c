#include "sys.h"
#include <assert.h>

// 定义 GPIO 端口到 EXTI 配置代码的转换宏
#define GPIO_PORT_TO_EXTI_CODE(port) ( \
    ((port) == GPIOA) ? 0x00 : \
    ((port) == GPIOB) ? 0x01 : \
    ((port) == GPIOC) ? 0x02 : \
    ((port) == GPIOD) ? 0x03 : \
    ((port) == GPIOE) ? 0x04 : \
    ((port) == GPIOF) ? 0x05 : \
    ((port) == GPIOG) ? 0x06 : \
    ((port) == GPIOH) ? 0x07 : \
    ((port) == GPIOI) ? 0x08 : 0x00)

// 参数验证宏
#define IS_GPIO_PORT(PORT) (((PORT) == GPIOA) || \
                            ((PORT) == GPIOB) || \
                            ((PORT) == GPIOC) || \
                            ((PORT) == GPIOD) || \
                            ((PORT) == GPIOE) || \
                            ((PORT) == GPIOF) || \
                            ((PORT) == GPIOG) || \
                            ((PORT) == GPIOH) || \
                            ((PORT) == GPIOI))

#define IS_GPIO_PIN(PIN)   ((PIN) < 16)  // 引脚号 0-15

#define IS_EXTI_TRIGGER(TRIGGER) (((TRIGGER) == EXTI_TRIGGER_FALLING) || \
                                  ((TRIGGER) == EXTI_TRIGGER_RISING)  || \
                                  ((TRIGGER) == EXTI_TRIGGER_BOTH))

#define IS_NVIC_PRIORITY_GROUP(GROUP) (((GROUP) >= 0) && ((GROUP) <= 4))

/**
  * @brief  配置外部中断线
  * @param  gpioPort: GPIO端口 (GPIOA, GPIOB, ..., GPIOI)
  * @param  pin: 引脚号 (0-15)
  * @param  triggerMode: 触发模式 (@ref EXTI_TriggerMode)
  * @retval None
  * @note   该函数会自动开启对应中断线和屏蔽线
  *         一次只能配置1个IO口，多个IO口需多次调用
  */
void EXTI_Config(GPIO_TypeDef* gpioPort, uint16_t pin, EXTI_TriggerMode triggerMode)
{
    // 计算EXTICR寄存器索引和偏移
    uint8_t extiRegIndex = pin / 4;
    uint8_t extiRegOffset = (pin % 4) * 4;
    
    // 使能SYSCFG时钟
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    
    // 配置GPIO源到EXTI线
    SYSCFG->EXTICR[extiRegIndex] &= ~(0x0F << extiRegOffset);
    SYSCFG->EXTICR[extiRegIndex] |= (GPIO_PORT_TO_EXTI_CODE(gpioPort) << extiRegOffset);
    
    // 开启中断线
    EXTI->IMR |= (1U << pin);
    
    // 配置触发模式
    EXTI->RTSR &= ~(1U << pin);  // 清除上升沿触发
    EXTI->FTSR &= ~(1U << pin);  // 清除下降沿触发
    
    if(triggerMode & EXTI_TRIGGER_RISING) {
        EXTI->RTSR |= (1U << pin);
    }
    if(triggerMode & EXTI_TRIGGER_FALLING) {
        EXTI->FTSR |= (1U << pin);
    }
}

/**
  * @brief  设置NVIC优先级分组
  * @param  priorityGroup: 优先级分组 (0-4)
  * @retval None
  */
void NVIC_PriorityGroupConfig(uint8_t priorityGroup)
{
    // 设置优先级分组
    NVIC_SetPriorityGrouping(priorityGroup);
}

/**
  * @brief  初始化中断向量
  * @param  irqNumber: 中断号 (如 EXTI0_IRQn)
  * @param  preemptPriority: 抢占优先级 (0-15)
  * @param  subPriority: 子优先级 (0-15)
  * @retval None
  */
void NVIC_Init(IRQn_Type irqNumber, uint8_t preemptPriority, uint8_t subPriority)
{
    // 设置中断优先级
    NVIC_SetPriority(irqNumber, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 
                                                    preemptPriority, 
                                                    subPriority));
    
    // 使能中断
    NVIC_EnableIRQ(irqNumber);
}

/**
  * @brief  GPIO复用功能设置（支持多引脚同时设置）
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PIN: 引脚掩码 (0x0000~0xFFFF)，每位对应一个引脚
  * @param  AFx: 复用功能选择 (0~15)
  * @note   复用功能的具体含义:
  *         AF0: MCO/SWD/SWCLK/RTC
  *         AF1: TIM1/TIM2
  *         AF2: TIM3~5
  *         AF3: TIM8~11
  *         AF4: I2C1~I2C3
  *         AF5: SPI1/SPI2
  *         AF6: SPI3
  *         AF7: USART1~3
  *         AF8: USART4~6
  *         AF9: CAN1/CAN2/TIM12~14
  *         AF10: USB_OTG/USB_HS
  *         AF11: ETH
  *         AF12: FSMC/SDIO/OTG/HS
  *         AF13: DCIM
  *         AF14: 保留
  *         AF15: EVENTOUT
  * @note 使用前需确保引脚已配置为复用模式
  */
void GPIO_AF(GPIO_TypeDef* GPIOx, uint32_t PIN, uint8_t AFx)
{
    // 参数有效性检查
    if(GPIOx == NULL) return;
    if((PIN & 0xFFFF) == 0) return;  // 无有效引脚
    
    // 确保AFx值在合法范围内
    AFx &= 0x0F;
    
    // 使用位带操作确保原子性
    volatile uint32_t* AFR[2] = {&GPIOx->AFR[0], &GPIOx->AFR[1]};
    
    // 同时处理低8位引脚 (AFR[0])
    uint32_t low_pins = PIN & 0x00FF;
    if(low_pins) {
        uint32_t temp = *AFR[0];
        for(uint8_t pinpos = 0; pinpos < 8; pinpos++) {
            if(low_pins & (1U << pinpos)) {
                uint8_t offset = pinpos * 4;
                temp &= ~(0x0FUL << offset);
                temp |= (uint32_t)AFx << offset;
            }
        }
        *AFR[0] = temp;
    }
    
    // 同时处理高8位引脚 (AFR[1])
    uint32_t high_pins = (PIN >> 8) & 0x00FF;
    if(high_pins) {
        uint32_t temp = *AFR[1];
        for(uint8_t pinpos = 0; pinpos < 8; pinpos++) {
            if(high_pins & (1U << pinpos)) {
                uint8_t offset = pinpos * 4;
                temp &= ~(0x0FUL << offset);
                temp |= (uint32_t)AFx << offset;
            }
        }
        *AFR[1] = temp;
    }
}

/**
  * @brief  设置GPIO引脚状态
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PIN: 引脚掩码 (0x0000~0xFFFF)，每位对应一个引脚
  * @param  status: 引脚状态 (0: 低电平, 其他值: 高电平)
  * @note   使用BSRR寄存器实现原子操作，支持多引脚同时设置
  */
void GPIO_Set(GPIO_TypeDef* GPIOx, uint16_t PIN, uint8_t status)
{
    // 参数有效性检查
    if(GPIOx && PIN) {
        // 使用三元运算符直接设置BSRR寄存器
        GPIOx->BSRR = status ? (uint32_t)PIN : ((uint32_t)PIN << 16);
    }
}

/**
  * @brief  读取GPIO引脚状态
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PIN: 引脚掩码 (0x0000~0xFFFF)，每位对应一个引脚
  * @retval 0: 所有选中引脚均为低电平
  *         1: 至少有一个选中引脚为高电平
  * @note   对于多引脚掩码，返回的是引脚状态的逻辑或(OR)结果
  */
uint8_t GPIO_Get(GPIO_TypeDef* GPIOx, uint16_t PIN)
{
    // 参数有效性检查
    if(!GPIOx || !PIN) return 0;
    
    // 直接返回逻辑判断结果（无分支）
    return (GPIOx->IDR & PIN) ? 1 : 0;
}

/**
  * @brief  GPIO初始化配置函数 
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PIN: 引脚掩码 (0x0000~0xFFFF)，每位对应一个引脚
  * @param  MODE: 模式设置，使用以下宏定义：
  *         @arg GPIO_Mode_IN   (0) 普通输入模式
  *         @arg GPIO_Mode_OUT  (1) 普通输出模式
  *         @arg GPIO_Mode_AF   (2) 复用功能模式
  *         @arg GPIO_Mode_Ain  (3) 模拟输入模式
  * @param  OTYPE: 输出类型，使用以下宏定义：
  *         @arg GPIO_Otype_PP  (0) 推挽输出
  *         @arg GPIO_Otype_OD  (1) 开漏输出
  * @param  OSPEED: 输出速度，使用以下宏定义：
  *         @arg GPIO_Speed_2M   (0) 低速 (2MHz)
  *         @arg GPIO_Speed_25M  (1) 中速 (25MHz)
  *         @arg GPIO_Speed_50M  (2) 高速 (50MHz)
  *         @arg GPIO_Speed_100M (3) 超高速 (100MHz)
  * @param  PUPD: 上拉/下拉设置，使用以下宏定义：
  *         @arg GPIO_PuPd_NOPULL (0) 无上拉/下拉
  *         @arg GPIO_PuPd_PU     (1) 上拉
  *         @arg GPIO_PuPd_PD     (2) 下拉
  *         @arg GPIO_PuPd_RES    (3) 保留
  * @note 1. 输入模式下(普通输入/模拟输入)忽略OTYPE和OSPEED参数
  *       2. 自动计算并启用对应GPIO的时钟
  */
void GPIO_Init(GPIO_TypeDef* GPIOx, uint32_t PIN,
               uint32_t MODE, uint32_t OTYPE, uint32_t OSPEED, uint32_t PUPD)
{
    // 自动计算并设置GPIO时钟使能位
    uint32_t gpio_index = ((uint32_t)GPIOx - GPIOA_BASE) / (GPIOB_BASE - GPIOA_BASE);
    uint32_t rcc_en_bit = RCC_AHB1ENR_GPIOAEN << gpio_index;
    
    // 使能GPIO时钟
    RCC->AHB1ENR |= rcc_en_bit;
    
    // 确保时钟使能完成 (读操作同步)
    __IO uint32_t tmp = RCC->AHB1ENR;
    (void)tmp;

    // 批量读取当前寄存器值
    uint32_t moder_val   = GPIOx->MODER;
    uint32_t otyper_val  = GPIOx->OTYPER;
    uint32_t ospeedr_val = GPIOx->OSPEEDR;
    uint32_t pupdr_val   = GPIOx->PUPDR;
    
    // 处理所有选中的引脚
    for (uint8_t pinpos = 0; pinpos < 16; pinpos++) {
        if (PIN & (1U << pinpos)) {
            const uint32_t pin_shift = pinpos * 2;
            
            // 配置模式寄存器
            moder_val &= ~(3UL << pin_shift);
            moder_val |= (MODE & 3) << pin_shift;
            
            // 仅在输出模式下配置速度和输出类型
            if (MODE == 1 || MODE == 2) { // 输出或复用模式
                ospeedr_val &= ~(3UL << pin_shift);
                ospeedr_val |= (OSPEED & 3) << pin_shift;
                
                otyper_val &= ~(1UL << pinpos);
                otyper_val |= (OTYPE & 1) << pinpos;
            }
            
            // 配置上拉/下拉寄存器
            pupdr_val &= ~(3UL << pin_shift);
            pupdr_val |= (PUPD & 3) << pin_shift;
        }
    }
    
    // 批量写入寄存器
    GPIOx->MODER   = moder_val;
    GPIOx->OSPEEDR = ospeedr_val;
    GPIOx->OTYPER  = otyper_val;
    GPIOx->PUPDR   = pupdr_val;
}

/**
  * @brief  进入等待中断模式 (WFI)
  * @note   使用内联汇编实现WFI指令
  */
__attribute__((always_inline)) 
inline void WFI_SET(void)
{
    __asm volatile ("wfi");
}

/**
  * @brief  禁用所有可屏蔽中断
  * @note   使用内联汇编实现CPSID I指令
  */
__attribute__((always_inline)) 
inline void INTX_DISABLE(void)
{
    __asm volatile ("cpsid i" : : : "memory");
}

/**
  * @brief  启用所有可屏蔽中断
  * @note   使用内联汇编实现CPSIE I指令
  */
__attribute__((always_inline)) 
inline void INTX_ENABLE(void)
{
    __asm volatile ("cpsie i" : : : "memory");
}

/**
  * @brief  设置主堆栈指针(MSP)
  * @param  addr: 栈顶地址 (必须8字节对齐)
  */
__attribute__((always_inline)) 
inline void MSR_MSP(uint32_t addr)
{
    // 检查地址对齐 (MSP必须8字节对齐)
    if((addr & 0x7) == 0) {
        // 使用内联汇编直接设置MSP
        __asm volatile ("MSR msp, %0" : : "r" (addr) : );
    }
    // 可选：添加错误处理
    // else { /* 处理对齐错误 */ }
}

/**
  * @brief  进入待机模式
  * @note   待机模式是最低功耗模式，唤醒后系统复位
  * @note   唤醒方式: WKUP引脚上升沿
  */
void Standby(void)
{
    // 1. 使能电源接口时钟
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    
    // 2. 确保寄存器写入完成
    __DSB();
    
    // 3. 配置唤醒源和电源控制
    PWR->CR |= PWR_CR_CWUF;          // 清除唤醒标志
    PWR->CR |= PWR_CR_CSBF;          // 清除待机标志 (可选)
    PWR->CR |= PWR_CR_PDDS;          // 进入待机模式时断电
    PWR->CSR |= PWR_CSR_EWUP;        // 使能WKUP引脚唤醒
    
    // 4. 设置深度睡眠模式
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    
    // 5. 确保所有内存操作完成
    __DSB();
    __ISB();
    
    // 6. 进入待机模式
    WFI_SET();
    
    // 7. 唤醒后执行系统复位 (代码不会执行到这里)
    NVIC_SystemReset();
}

/**
  * @brief  软件复位系统
  * @note   执行后系统立即复位
  */
__attribute__((noreturn)) 
void Soft_Reset(void)
{
    // 1. 确保所有内存操作完成
    __DSB();
    
    // 2. 请求系统复位
    SCB->AIRCR = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos)  // 密钥
               | SCB_AIRCR_SYSRESETREQ_Msk;         // 复位请求
    
    // 3. 确保复位请求完成
    __DSB();
    
    // 4. 等待复位发生
    for(;;) {
        __NOP();
    }
}


/**
  * @brief  系统时钟配置函数
  * @param  plln: PLL倍频系数 (4-512)
  * @param  pllm: PLL预分频系数 (2-63)
  * @param  pllp: PLL P分频系数 (2-128, 必须偶数)
  * @param  pllq: PLL Q分频系数 (1-128)
  * @retval 0: 成功, 1: 失败
  * @note   公式:
  *         Fvco = HSE_FREQ * (plln / pllm)
  *         Fsys = Fvco / pllp
  *         Fq   = Fvco / pllq
  *         推荐值 (25MHz HSE): plln=160, pllm=5, pllp=2, pllq=4
  */
uint8_t Sys_Clock_Config(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    // 参数有效性检查
    if(pllm < 2 || pllm > 63)   return 1;
    if(plln < 4 || plln > 512)  return 1;
    if(pllp < 2 || pllp > 128 || (pllp & 1)) return 1;
    if(pllq < 1 || pllq > 128)  return 1;
    
    // 1. 使能HSE并等待就绪
    RCC->CR |= RCC_CR_HSEON;
    
    // 等待HSE就绪 (最长约500ms @ 25MHz)
    uint32_t timeout = 500000; // 根据系统时钟调整
    while(!(RCC->CR & RCC_CR_HSERDY)) {
        if(--timeout == 0) return 1; // HSE启动失败
    }
    
    // 2. 配置电源和Flash
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // 使能电源时钟
    PWR->CR |= PWR_CR_VOS;              // 高性能模式 (VOS = Scale 1)
    
    // Flash等待周期 (根据目标系统时钟设置)
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;
    
    // 3. 配置总线分频
    RCC->CFGR &= ~(RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk);
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1   |  // HCLK不分频
                RCC_CFGR_PPRE1_DIV4  |  // APB1 4分频 (100MHz max)
                RCC_CFGR_PPRE2_DIV2;    // APB2 2分频 (100MHz max)
    
    // 4. 配置主PLL
    RCC->CR &= ~RCC_CR_PLLON;           // 关闭PLL
    while(RCC->CR & RCC_CR_PLLRDY);     // 等待PLL停止
    
    // 计算PLLP寄存器值 (实际值 = (pllp/2)-1)
    uint32_t pllp_reg = ((pllp >> 1) - 1) << RCC_PLLCFGR_PLLP_Pos;
    
    RCC->PLLCFGR = (pllm << RCC_PLLCFGR_PLLM_Pos) |
                   (plln << RCC_PLLCFGR_PLLN_Pos) |
                   pllp_reg |
                   (pllq << RCC_PLLCFGR_PLLQ_Pos) |
                   RCC_PLLCFGR_PLLSRC_HSE; // HSE作为PLL源
    
    RCC->CR |= RCC_CR_PLLON;            // 开启PLL
    
    // 等待PLL锁定
    timeout = 500000;
    while(!(RCC->CR & RCC_CR_PLLRDY)) {
        if(--timeout == 0) return 1;    // PLL锁定失败
    }
    
    // 5. 切换到PLL系统时钟
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    
    // 等待时钟切换完成
    timeout = 500000;
    while((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL) {
        if(--timeout == 0) return 1;    // 时钟切换失败
    }
    
    // 6. 更新SystemCoreClock变量 (如果使用CMSIS)
    // SystemCoreClockUpdate();
    
    return 0; // 成功
}


/**
  * @brief  设置向量表偏移地址
  * @param  baseAddr: 向量表基地址 (必须对齐到向量表大小)
  * @param  offset:   向量表偏移量 (相对于基地址)
  * @note   根据Cortex-M内核要求，向量表地址必须对齐到其大小
  *         函数确保满足VTOR寄存器的对齐要求
  */
void NVIC_SetVectorTable(uint32_t baseAddr, uint32_t offset)
{
    /* VTOR寄存器要求地址对齐到512字节边界 (低9位保留) */
    const uint32_t VTOR_ALIGNMENT_MASK = 0xFFFFFE00U;
    
    /* 组合基地址和偏移量，并确保地址对齐 */
    SCB->VTOR = (baseAddr | offset) & VTOR_ALIGNMENT_MASK;
}

/**
  * @brief  系统时钟初始化函数
  * @param  plln: PLL倍频系数 (主PLL倍频因子)
  * @param  pllm: PLL分频系数 (主PLL和音频PLL分频因子)
  * @param  pllp: 系统时钟分频系数 (主PLL分频因子)
  * @param  pllq: USB/SDIO/RNG时钟分频系数
  * @note   配置系统时钟源和PLL参数，使用寄存器直接操作
  */
void Sys_Clock_Init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
  // 1. 启用内部高速时钟 (HSI)
  RCC->CR |= RCC_CR_HSION;
  
  // 2. 复位时钟配置寄存器
  RCC->CFGR = 0x00000000;
  
  // 3. 清除关键时钟控制位
  RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);
  
  // 4. 恢复PLL配置寄存器为复位值
  RCC->PLLCFGR = 0x24003010;
  
  // 5. 禁用外部时钟旁路
  RCC->CR &= ~RCC_CR_HSEBYP;
  
  // 6. 禁用所有时钟中断
  RCC->CIR = 0x00000000;
  
  // 7. 配置系统时钟树
  Sys_Clock_Config(plln, pllm, pllp, pllq);

  // 8. 配置向量表位置
#ifdef VECT_TAB_RAM
    // 如果向量表在RAM中
  NVIC_SetVectorTable(D1_AXISRAM_BASE, 0x0);
#else
  // 如果向量表在FLASH中
  NVIC_SetVectorTable(FLASH_BASE, 0x0);
#endif
    
  // 8. 等待时钟稳定
  while((RCC->CR & RCC_CR_HSERDY) == 0);  // 等待HSE就绪
  while((RCC->CR & RCC_CR_PLLRDY) == 0);  // 等待PLL1就绪
}



