#include "sys.h"

/**
  * @brief  配置外部中断线
  * @param  GPIOx: GPIO端口 (0~8 对应 GPIOA~GPIOI)
  * @param  BITx: 引脚位 (0~15)
  * @param  TRIM: 触发模式 (1:下降沿, 2:上升沿, 3:任意边沿)
  * @retval None
  * @note   该函数会自动开启对应中断线和屏蔽线
  *         一次只能配置1个IO口，多个IO口需多次调用
  *         仅支持GPIOA~I，不包括PVD/RTC/USB/以太网等外设中断
  */
void Ex_Config(uint8_t GPIOx, uint8_t BITx, uint8_t TRIM)
{
    uint8_t EXTOFFSET = (BITx % 4) * 4;
    uint8_t EXTICR_index = BITx / 4;
    
    // 使能SYSCFG时钟
    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;
    
    // 配置GPIO源到EXTI线
    SYSCFG->EXTICR[EXTICR_index] &= ~(0x0FUL << EXTOFFSET);
    SYSCFG->EXTICR[EXTICR_index] |= (GPIOx << EXTOFFSET);
    
    // 开启中断线
    EXTI_D1->IMR1 |= (1UL << BITx);
    
    // 配置触发模式
    if(TRIM & 0x01) EXTI->FTSR1 |= (1UL << BITx);  // 下降沿触发
    if(TRIM & 0x02) EXTI->RTSR1 |= (1UL << BITx);  // 上升沿触发
}

/**
  * @brief  设置NVIC优先级分组
  * @param  NVIC_Group: 优先级分组 (0~4)
  * @retval None
  */
void NVIC_PriorityGroupConfig(uint8_t NVIC_Group)
{
    uint32_t temp = SCB->AIRCR;
    
    // 清除先前分组设置
    temp &= ~(SCB_AIRCR_VECTKEYSTAT_Msk | SCB_AIRCR_PRIGROUP_Msk);
    
    // 设置新分组
    temp |= (0x05FAUL << SCB_AIRCR_VECTKEY_Pos) | 
            ((uint32_t)NVIC_Group << SCB_AIRCR_PRIGROUP_Pos);
    
    SCB->AIRCR = temp;
}

/**
  * @brief  初始化中断向量
  * @param  X_IRQn: 中断号 (如 EXTI0_IRQn)
  * @param  NVIC_PreemptionPriority: 抢占优先级 (0-15)
  * @param  NVIC_SubPriority: 子优先级 (0-15)
  * @param  NVIC_Group: 优先级分组 (0-4)
  * @retval None
  */
void NVIC_Init(uint8_t NVIC_PreemptionPriority, 
                 uint8_t NVIC_SubPriority, 
                 IRQn_Type X_IRQn, 
                 uint8_t NVIC_Group)
{
    // 设置优先级分组
    NVIC_PriorityGroupConfig(NVIC_Group);
    
    // 计算优先级值
    uint32_t priority = NVIC_PreemptionPriority << (4 - NVIC_Group);
    priority |= NVIC_SubPriority & (0x0F >> NVIC_Group);
    priority &= 0x0F;  // 确保4位有效
    
    // 设置中断优先级
    NVIC_SetPriority(X_IRQn, priority);
    
    // 使能中断
    NVIC_EnableIRQ(X_IRQn);
}

/**
  * @brief  GPIO复用功能设置（支持多引脚同时设置）
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PinMask: 引脚掩码 (0x0001~0xFFFF)，每位对应一个引脚(0~15)
  * @param  AFx: 复用功能选择 (0~15)
  * @note   复用功能的具体含义:
  *         AF0: MCO/SWD/SWCLK/RTC
  *         AF1: TIM1/2/TIM16/17/LPTIM1
  *         AF2: TIM3~5/TIM12/HRTIM1/SAI1
  *         AF3: TIM8/LPTIM2~5/HRTIM1/LPUART1
  *         AF4: I2C1~I2C4/TIM15/USART1
  *         AF5: SPI1~SPI6/CEC
  *         AF6: SPI3/SAI1~3/UART4/I2C4
  *         AF7: SPI2/3/6/USART1~3/6/UART7/SDIO1
  *         AF8: USART4/5/8/SPDIF/SAI2/4
  *         AF9: FDCAN1~2/TIM13/14/LCD/QSPI
  *         AF10: USB_OTG1/2/SAI2/4/QSPI
  *         AF11: ETH/UART7/SDIO2/I2C4/COMP1/2
  *         AF12: FMC/SDIO1/OTG2/LCD
  *         AF13: DCIM/DSI/LCD/COMP1/2
  *         AF14: LCD/UART5
  *         AF15: EVENTOUT
  * @note 使用前需确保引脚已配置为复用模式，具体复用功能请参考STM32H743xx数据手册
  * @note 支持同时设置多个引脚复用功能
  */
void GPIO_AF(GPIO_TypeDef* GPIOx, uint32_t PinMask, uint8_t AFx)
{
    uint32_t af_value = AFx & 0x0F;  // 确保复用值在0-15范围
    uint32_t afr[2] = {GPIOx->AFR[0], GPIOx->AFR[1]}; // 一次性读取两个复用寄存器
    
    // 处理低8位引脚 (AFR[0])
    for(uint32_t pin = 0; pin < 8; pin++) {
        if(PinMask & (1UL << pin)) {
            const uint32_t shift = pin * 4;
            const uint32_t mask = 0x0FUL << shift;
            afr[0] = (afr[0] & ~mask) | (af_value << shift);
        }
    }
    
    // 处理高8位引脚 (AFR[1])
    for(uint32_t pin = 8; pin < 16; pin++) {
        if(PinMask & (1UL << pin)) {
            const uint32_t shift = (pin - 8) * 4;
            const uint32_t mask = 0x0FUL << shift;
            afr[1] = (afr[1] & ~mask) | (af_value << shift);
        }
    }
    
    // 批量写回寄存器
    GPIOx->AFR[0] = afr[0];
    GPIOx->AFR[1] = afr[1];
}

/**
  * @brief  设置GPIO引脚状态
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PIN: 引脚掩码 (0x0000~0xFFFF)，每位对应一个引脚
  * @param  status: 引脚状态 (0: 低电平, 其他值: 高电平)
  * @note   1. 使用BSRR寄存器实现原子操作，避免读-修改-写操作中的竞态条件
  *         2. 支持多引脚同时设置，所有选中引脚将设置为相同状态
  *         3. BSRR寄存器特性：
  *            - 低16位(BSRRL): 置1对应引脚为高电平
  *            - 高16位(BSRRH): 置1对应引脚为低电平
  *         4. STM32 M7系列无位带操作单元，必须使用BSRR寄存器控制GPIO状态
  */
void GPIO_Set(GPIO_TypeDef* GPIOx, uint16_t PIN, uint8_t status)
{
    // 参数有效性检查
    if(GPIOx == NULL || PIN == 0) return;
    
    // 使用单条指令设置所有选中引脚状态
    if(status & 0x01) {
        GPIOx->BSRRL = PIN;  // 设置选中引脚为高电平
    } else {
        GPIOx->BSRRH = PIN;  // 设置选中引脚为低电平
    }
}

/**
  * @brief  读取GPIO引脚状态
  * @param  GPIOx: GPIO端口 (GPIOA~GPIOK)
  * @param  PIN: 引脚掩码 (0x0000~0xFFFF)，每位对应一个引脚
  * @retval 0: 所有选中引脚均为低电平
  *         1: 至少有一个选中引脚为高电平
  * @note   1. 对于多引脚掩码，返回的是引脚状态的逻辑或(OR)结果
  *         2. STM32 M7系列无位带操作单元，必须直接读取IDR寄存器
  *         3. 如需获取单个引脚状态，应传入单个引脚掩码(如GPIO_PIN_5)
  */
uint8_t GPIO_Get(GPIO_TypeDef* GPIOx, uint16_t PIN)
{
    // 参数有效性检查
    if(GPIOx == NULL || PIN == 0) return 0;
    
    // 读取IDR寄存器并检查选中引脚状态
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
               GPIO_ModeTypeDef MODE, GPIO_OTypeTypeDef OTYPE,
               GPIO_SpeedTypeDef OSPEED, GPIO_PuPdTypeDef PUPD)
{
    // 计算GPIO端口索引 (0=GPIOA, 1=GPIOB, ...)
    uint32_t gpio_index = ((uint32_t)GPIOx - GPIOA_BASE) / (GPIOB_BASE - GPIOA_BASE);
    // 计算对应的时钟使能位
    uint32_t rcc_en_bit = RCC_AHB4ENR_GPIOAEN << gpio_index;
    
    // 使能GPIO时钟 (AHB4总线)
    RCC->AHB4ENR |= rcc_en_bit;
    
    // 确保时钟使能完成
    __DSB();

    // 批量读取当前寄存器值
    uint32_t moder_val   = GPIOx->MODER;
    uint32_t otyper_val  = GPIOx->OTYPER;
    uint32_t ospeedr_val = GPIOx->OSPEEDR;
    uint32_t pupdr_val   = GPIOx->PUPDR;
    
    // 处理所有选中的引脚
    for (uint8_t pinpos = 0; pinpos < 16; pinpos++) {
        if (PIN & (1U << pinpos)) {
            const uint32_t pin_shift = pinpos * 2;
            
            // MODER配置: 清除旧值并设置新值
            moder_val &= ~(3UL << pin_shift);
            moder_val |= (MODE << pin_shift);
            
            // 仅输出/复用模式配置速度和输出类型
            if (MODE == GPIO_MODE_OUT || MODE == GPIO_MODE_AF) {
                // OSPEEDR配置
                ospeedr_val &= ~(3UL << pin_shift);
                ospeedr_val |= (OSPEED << pin_shift);
                
                // OTYPER配置
                if (OTYPE == GPIO_OTYPE_OD) {
                    otyper_val |= (1UL << pinpos);  // 开漏输出
                } else {
                    otyper_val &= ~(1UL << pinpos); // 推挽输出
                }
            }
            
            // PUPDR配置
            pupdr_val &= ~(3UL << pin_shift);
            pupdr_val |= (PUPD << pin_shift);
        }
    }
    
    // 批量写入寄存器
    GPIOx->MODER   = moder_val;
    GPIOx->OTYPER  = otyper_val;
    GPIOx->OSPEEDR = ospeedr_val;
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
  * @note   1. 待机模式下芯片完全断电，仅保留备份域供电
  *         2. 唤醒后系统将执行完整复位
  *         3. 仅支持WKUP1~6引脚唤醒（PA0~PA5或PC13等）
  *         4. 唤醒后所有寄存器恢复复位值（除备份域寄存器）
  */
void Standby(void)
{
    // 1. 使能电源接口时钟
    RCC->APB4ENR |= (1U << 0);  // PWR时钟使能 (bit0)
    
    // 2. 确保寄存器写入完成
    __DSB();
    
    // 3. 配置唤醒源 (这里配置WKUP1 - PA0)
    PWR->WKUPEPR |= (1U << 0);    // 使能WKUP1唤醒功能 (bit0)
    PWR->WKUPEPR &= ~(1U << 8);   // WKUPP1=0: 上升沿唤醒 (bit8)
    
    // 4. 配置唤醒引脚内部电阻
    PWR->WKUPEPR &= ~(0x3U << 2); // 清除WKUPPUPD1设置 (bit[2:3])
    PWR->WKUPEPR |=  (0x2U << 2); // WKUPPUPD1=2: 内部下拉 (bit[2:3]=10b)
    
    // 5. 清除所有唤醒标志 (WKUP1~6)
    PWR->WKUPCR = (1U << 0) |    // WKUPC1: 清除WKUP1标志
                  (1U << 1) |    // WKUPC2: 清除WKUP2标志
                  (1U << 2) |    // WKUPC3: 清除WKUP3标志
                  (1U << 3) |    // WKUPC4: 清除WKUP4标志
                  (1U << 4) |    // WKUPC5: 清除WKUP5标志
                  (1U << 5);     // WKUPC6: 清除WKUP6标志
    
    // 6. 配置电源域深度睡眠模式
    PWR->CR3 |= (1U << 16) | (1U << 17) | (1U << 18);  // 使能SD1/SD2/SD3
    
    // 7. 设置待机模式
    PWR->CR1 &= ~(0x7U << 0);  // 先清除LPMS位
    PWR->CR1 |= (0x3U << 0);   // LPMS=011: 待机模式 (bit0=1, bit1=1)
    
    // 8. 设置深度睡眠模式
    SCB->SCR |= (1U << 2);  // 使能SLEEPDEEP位
    
    // 9. 确保所有内存操作完成
    __DSB();
    __ISB();
    
    // 10. 进入待机模式
    __WFI();  // 执行WFI指令
    
    // 唤醒后系统自动复位
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
//#因为在写程序时发现要使能STM32H7的L1-Cache,同时开启D cache的强制透写
void Cache_Enable(void)
{
    SCB_EnableICache();	//使能I-Cache,函数在core_cm7.h里面定义
    SCB_EnableDCache();	//使能D-Cache,函数在core_cm7.h里面定义
	SCB->CACR|=1<<2;	//强制D-Cache透写,如不开启透写,实际使用中可能遇到各种问题
}

/**
  * @brief  系统时钟配置函数
  * @param  plln: PLL1倍频系数 (4-512)
  * @param  pllm: PLL1预分频系数 (2-63)
  * @param  pllp: PLL1的p分频系数 (2-128, 且为2的倍数)
  * @param  pllq: PLL1的q分频系数 (1-128)
  * @retval 0: 成功, 1: 失败
  * @note   1. 配置CPU时钟=400MHz, AHB总线=200MHz, APB总线=100MHz
  *         2. 同时配置PLL2用于FMC时钟(224MHz)
  */
uint8_t Sys_Clock_Config(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    // 参数有效性检查
    if((pllm < 2) || (pllm > 63) || (plln < 4) || (plln > 512) || 
       (pllp < 2) || (pllp > 128) || (pllp & 1) || (pllq < 1) || (pllq > 128)) {
        return 1; // 参数无效
    }

    // 1. 配置内核电压 (Scale1, 1.15~1.26V)
    PWR->CR3 &= ~PWR_CR3_SCUEN;           // 解锁电压配置
    PWR->D3CR = (PWR->D3CR & ~PWR_D3CR_VOS_Msk) | (3 << PWR_D3CR_VOS_Pos); // VOS=3
    while((PWR->D3CR & PWR_D3CR_VOSRDY) == 0); // 等待电压稳定

    // 2. 启用HSE时钟
    RCC->CR |= RCC_CR_HSEON;
    uint32_t retry = 500000; // 适当增加超时计数
    while(((RCC->CR & RCC_CR_HSERDY) == 0) && (--retry));
    if(retry == 0) return 1; // HSE启动失败

    // 3. 配置PLL1时钟源和分频系数
    RCC->PLLCKSELR = (RCC->PLLCKSELR & ~(RCC_PLLCKSELR_PLLSRC_Msk | RCC_PLLCKSELR_DIVM1_Msk)) 
                   | (2 << RCC_PLLCKSELR_PLLSRC_Pos)   // HSE作为PLL源
                   | (pllm << RCC_PLLCKSELR_DIVM1_Pos); // M分频
    
    RCC->PLL1DIVR = (plln - 1) << RCC_PLL1DIVR_N1_Pos  // N倍频
                  | (pllp - 1) << RCC_PLL1DIVR_P1_Pos  // P分频
                  | (pllq - 1) << RCC_PLL1DIVR_Q1_Pos  // Q分频
                  | (1)      << RCC_PLL1DIVR_R1_Pos;   // R分频(默认值)
    
    RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLL1RGE_Msk | RCC_PLLCFGR_PLL1VCOSEL_Msk)) 
                 | (2 << RCC_PLLCFGR_PLL1RGE_Pos)   // 输入频率范围4-8MHz
                 | (0 << RCC_PLLCFGR_PLL1VCOSEL_Pos)// VCO宽范围
                 | RCC_PLLCFGR_DIVP1EN             // 使能P输出
                 | RCC_PLLCFGR_DIVQ1EN;            // 使能Q输出
    
    RCC->CR |= RCC_CR_PLLON;      // 启动PLL1
    while((RCC->CR & RCC_CR_PLLRDY) == 0); // 等待PLL就绪

    // 4. 配置PLL2用于FMC时钟 (固定配置)
    RCC->PLLCKSELR = (RCC->PLLCKSELR & ~RCC_PLLCKSELR_DIVM2_Msk) 
                   | (25 << RCC_PLLCKSELR_DIVM2_Pos); // M分频=25
    
    RCC->PLL2DIVR = (448 - 1) << RCC_PLL2DIVR_N2_Pos  // N=448
                  | (2 - 1)   << RCC_PLL2DIVR_P2_Pos  // P=2
                  | (2 - 1)   << RCC_PLL2DIVR_R2_Pos; // R=2
    
    RCC->PLLCFGR |= (0 << RCC_PLLCFGR_PLL2RGE_Pos)   // 输入频率1-2MHz
                 | (0 << RCC_PLLCFGR_PLL2VCOSEL_Pos)// VCO宽范围
                 | RCC_PLLCFGR_DIVP2EN              // 使能P输出
                 | RCC_PLLCFGR_DIVR2EN;             // 使能R输出
    
    RCC->D1CCIPR = (RCC->D1CCIPR & ~RCC_D1CCIPR_FMCSEL_Msk) 
                 | (2 << RCC_D1CCIPR_FMCSEL_Pos); // FMC时钟源=PLL2_R
    
    RCC->CR |= RCC_CR_PLL2ON;     // 启动PLL2
    while((RCC->CR & RCC_CR_PLL2RDY) == 0); // 等待就绪

    // 5. 配置系统时钟分频
    RCC->D1CFGR = (RCC->D1CFGR & ~(RCC_D1CFGR_HPRE_Msk | RCC_D1CFGR_D1CPRE_Msk)) 
                | (8 << RCC_D1CFGR_HPRE_Pos)    // AHB预分频=2 (200MHz)
                | (0 << RCC_D1CFGR_D1CPRE_Pos); // CPU不分频 (400MHz)
    
    // 6. 切换系统时钟到PLL1
    RCC->CFGR |= RCC_CFGR_SW_PLL1; // 选择PLL1作为系统时钟
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1); // 等待切换完成

    // 7. 配置Flash等待周期 (400MHz需要2等待周期)
    FLASH->ACR = (FLASH->ACR & ~(FLASH_ACR_LATENCY_Msk | FLASH_ACR_WRHIGHFREQ_Msk)) 
               | (2 << FLASH_ACR_LATENCY_Pos)    // 2等待周期
               | (2 << FLASH_ACR_WRHIGHFREQ_Pos);// 频率<285MHz设置

    // 8. 配置APB分频 (100MHz)
    RCC->D1CFGR = (RCC->D1CFGR & ~RCC_D1CFGR_D1PPRE_Msk) 
                | (4 << RCC_D1CFGR_D1PPRE_Pos); // APB3分频=2
    RCC->D2CFGR = (RCC->D2CFGR & ~(RCC_D2CFGR_D2PPRE1_Msk | RCC_D2CFGR_D2PPRE2_Msk)) 
                | (4 << RCC_D2CFGR_D2PPRE1_Pos) // APB1分频=2
                | (4 << RCC_D2CFGR_D2PPRE2_Pos);// APB2分频=2
    RCC->D3CFGR = (RCC->D3CFGR & ~RCC_D3CFGR_D3PPRE_Msk) 
                | (4 << RCC_D3CFGR_D3PPRE_Pos); // APB4分频=2

    // 9. 使能IO补偿单元
    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN; // 使能SYSCFG时钟
    RCC->CR |= RCC_CR_CSION;              // 使能CSI时钟
    SYSCFG->CCCSR |= SYSCFG_CCCSR_EN;     // 使能IO补偿

    return 0; // 配置成功
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
  * @param  plln: PLL倍频系数 (主PLL倍频因子, 4-512)
  * @param  pllm: PLL分频系数 (主PLL预分频因子, 2-63)
  * @param  pllp: 系统时钟分频系数 (主PLL P分频因子, 2-128且为2的倍数)
  * @param  pllq: USB/SDIO/RNG时钟分频系数 (1-128)
  * @note   配置系统时钟源和PLL参数，使用寄存器直接操作
  */
void Sys_Clock_Init(u32 plln, u32 pllm, u32 pllp, u32 pllq)
{
    // 1. 启用内部高速时钟 (HSI) 作为临时时钟源
    RCC->CR = RCC_CR_HSION;  // 设置HISON, 开启内部高速RC振荡
    
    // 2. 复位所有时钟配置寄存器到默认状态
    RCC->CFGR = 0x00000000;       // 清除时钟配置
    RCC->D1CFGR = 0x00000000;     // 清除域1配置
    RCC->D2CFGR = 0x00000000;     // 清除域2配置
    RCC->D3CFGR = 0x00000000;     // 清除域3配置
    RCC->PLLCKSELR = 0x00000000;  // 清除PLL时钟源选择
    RCC->PLLCFGR = 0x00000000;    // 清除PLL配置
    
    // 3. 禁用所有时钟相关中断
    RCC->CIER = 0x00000000;       // 禁止所有RCC中断
    
    // 4. 禁用外部时钟和PLL
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSHSEON | RCC_CR_PLLON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);
    
    // 5. 禁用外部时钟旁路
    RCC->CR &= ~RCC_CR_HSEBYP;
    
    // 6. 配置AXI SRAM矩阵读取能力 (特定于STM32H7)
    *((vu32*)0x51008108) = 0x00000001;  // 设置AXI SRAM的矩阵读取能力为1
    
    // 7. 使能L1 Cache
    Cache_Enable();  // 使能L1 Cache以提高性能
    
    // 8. 配置系统时钟树
    Sys_Clock_Config(plln, pllm, pllp, pllq);  // 配置主时钟树
    
    // 9. 配置向量表位置
#ifdef VECT_TAB_RAM
    // 如果向量表在RAM中
    NVIC_SetVectorTable(D1_AXISRAM_BASE, 0x0);
#else
    // 如果向量表在FLASH中
    NVIC_SetVectorTable(FLASH_BANK1_BASE, 0x0);
#endif
    
    // 10. 等待时钟稳定
    while((RCC->CR & RCC_CR_HSERDY) == 0);  // 等待HSE就绪
    while((RCC->CR & RCC_CR_PLLRDY) == 0);  // 等待PLL1就绪
    while((RCC->CR & RCC_CR_PLL2RDY) == 0); // 等待PLL2就绪
}
