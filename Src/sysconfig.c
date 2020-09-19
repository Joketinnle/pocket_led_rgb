#include "includes.h"
#include "sysconfig.h"

extern osThreadId secnTaskHandle;
extern osThreadId dispTaskHandle;
extern osThreadId batteryTaskHandle;


static bool sys_stop_mode_flag = true;

void sys_show_info(void)
{
    printf("system starting...\r\n");
    printf("FW build %s  %s\r\n", __DATE__, __TIME__);
    printf("version %d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_NUM);
}


void sys_stop_mode_enable(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    
    /* disable ir exti */
    __HAL_RCC_GPIOC_IS_CLK_DISABLED();
    /* enable STAT_1 exti */
    GPIO_InitStruct.Pin = STAT_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(STAT_1_GPIO_Port, &GPIO_InitStruct);
    // HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    sys_stop_mode_flag = true;
    HAL_SuspendTick();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);




    /* second */
    /* enter stop mode */

}

void sys_stop_mode_disable(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = STAT_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(STAT_1_GPIO_Port, &GPIO_InitStruct); 

    /* enable GPIO_C for ir*/
    __HAL_RCC_GPIOC_IS_CLK_ENABLED();

    sys_stop_mode_flag = false;
    HAL_ResumeTick();
    HAL_NVIC_DisableIRQ(POWER_EXTI_IRQn);
    HAL_NVIC_DisableIRQ(POWER_EXTI_IRQn);

    SystemClock_Config();
}

bool is_sys_in_stop_mode(void)
{
    return sys_stop_mode_flag;
}

void sys_reset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}


