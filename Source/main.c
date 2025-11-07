/*!
 * @file        main.c
 *
 * @brief       Main program body
 *
 * @version     V1.0.3
 *
 * @date        2022-12-01
 *
 * @attention
 *
 *  Copyright (C) 2020-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

/* Includes */
#include "main.h"

#include <string.h>

#include "apm32f10x_fmc.h"
#include "usbd_cdc.h"
/** @addtogroup Examples
  @{
  */
uint8_t Tx_buffer[13000];
uint32_t start = 0;
uint16_t maxPoints=12500;
uint8_t sampComplete=1;
/** @addtogroup Template
  @{
  */

/** @defgroup Template_Functions
  @{
  */
void SetSysClock(void);

/*!
 * @brief       Main program
 *
 * @param       None
 *
 * @retval      None
 *
 */
int main(void) {
    SetSysClock();
    RCM_EnableCSS();
    GPIO_Config_T gpioConfig;
    EINT_Config_T eintConfig;
    // 使能 GPIOA 和 AFIO 时钟
    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_GPIOA | RCM_APB2_PERIPH_AFIO);
    gpioConfig.pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
    gpioConfig.mode = GPIO_MODE_IN_PU;  // 上拉
    GPIO_Config(GPIOA, &gpioConfig);
    GPIO_ConfigEINTLine(GPIO_PORT_SOURCE_A, GPIO_PIN_SOURCE_0);
    GPIO_ConfigEINTLine(GPIO_PORT_SOURCE_A, GPIO_PIN_SOURCE_1);
    GPIO_ConfigEINTLine(GPIO_PORT_SOURCE_A, GPIO_PIN_SOURCE_2);
    // 配置 EXTI0,1,2：中断模式 + 双边沿触发
    eintConfig.line = EINT_LINE_0|EINT_LINE_1|EINT_LINE_2;
    eintConfig.mode = EINT_MODE_INTERRUPT;
    eintConfig.trigger = EINT_TRIGGER_RISING_FALLING;  // 电平变化就触发（上升+下降沿）
    eintConfig.lineCmd = ENABLE;
    EINT_Config(&eintConfig);

    // 使能 EXTI1 中断(PA0,1,2 对应 EXTI0,1,2，使用 EXTI0,1,2_IRQn）
    NVIC_EnableIRQRequest(EINT0_IRQn, 0x02, 0x02);
    NVIC_EnableIRQRequest(EINT1_IRQn, 0x02, 0x02);
    NVIC_EnableIRQRequest(EINT2_IRQn, 0x02, 0x02);

    // 禁用中断,等待上位机发来命令
    NVIC_DisableIRQ(EINT0_IRQn);
    NVIC_DisableIRQ(EINT1_IRQn);
    NVIC_DisableIRQ(EINT2_IRQn);

    // 使能DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 初始化USB虚拟串口
    CDC_Init();

    while (1) {
        while (start<maxPoints);
        //达到最大点数禁用中断,防止点数继续增加
        NVIC_DisableIRQ(EINT0_IRQn);
        NVIC_DisableIRQ(EINT1_IRQn);
        NVIC_DisableIRQ(EINT2_IRQn);
        //清除可能的中断标志位
        EINT_ClearIntFlag(EINT_LINE_0|EINT_LINE_1|EINT_LINE_2);
        //发送获得的数据
        USBD_TxData(USBD_EP_1,Tx_buffer, maxPoints);
        //清零变量,等待下一次采集
        start=0;
        sampComplete=1;
    }
}
static void recordPoint() {
    //if (start>=maxPoints)return;
    *((uint32_t *) (Tx_buffer + start + 1)) = DWT->CYCCNT;
    Tx_buffer[start] = (GPIOA->IDATA) | 0b11111000;
    start += 5;
}
void EINT0_IRQHandler(void)
{
    if(EINT_ReadIntFlag(EINT_LINE_0))
    {
        recordPoint();
        EINT_ClearIntFlag(EINT_LINE_0);
    }
}
void EINT1_IRQHandler(void)
{
    if(EINT_ReadIntFlag(EINT_LINE_1))
    {
        recordPoint();
        EINT_ClearIntFlag(EINT_LINE_1);
    }
}
void EINT2_IRQHandler(void)
{
    if(EINT_ReadIntFlag(EINT_LINE_2))
    {
        recordPoint();
        EINT_ClearIntFlag(EINT_LINE_2);
    }
}
void SetSysClock(void) {
    RCM_Reset();
    RCM_ConfigHSE(RCM_HSE_OPEN);

    if (RCM_WaitHSEReady() == SUCCESS) {
        FMC_EnablePrefetchBuffer();
        FMC_ConfigLatency(FMC_LATENCY_2);

        RCM_ConfigAHB(RCM_AHB_DIV_1);
        RCM_ConfigAPB2(RCM_APB_DIV_1);
        RCM_ConfigAPB1(RCM_APB_DIV_2);

        RCM_ConfigPLL(RCM_PLLSEL_HSE, RCM_PLLMF_9);
        RCM_EnablePLL();
        while (RCM_ReadStatusFlag(RCM_FLAG_PLLRDY) == RESET);

        RCM_ConfigSYSCLK(RCM_SYSCLK_SEL_PLL);
        while (RCM_ReadSYSCLKSource() != RCM_SYSCLK_SEL_PLL);
    } else {
        while (1);
    }
}

/**@} end of group Template_Functions */
/**@} end of group Template */
/**@} end of group Examples */
