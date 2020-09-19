#include "includes.h"
#include "sysconfig.h"

extern osThreadId secnTaskHandle;
extern osThreadId dispTaskHandle;
extern osThreadId batteryTaskHandle;


void sys_show_info(void)
{
    printf("system starting...\r\n");
    printf("FW build %s  %s\r\n", __DATE__, __TIME__);
    printf("version %d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_NUM);
}


void sys_stop_mode_enable(void)
{
    /* first */
    /* enable GPIO EXTI(charging pin and power button) */
    
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_SuspendTick();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    

    /* second */
    /* enter stop mode */

}

void sys_stop_mode_disable(void)
{
    HAL_ResumeTick();
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
    SystemClock_Config();
}


void sys_suspend_all_task(void)
{
    osThreadSuspend(secnTaskHandle);
    osThreadSuspend(dispTaskHandle);
    osThreadSuspend(batteryTaskHandle);
}

void sys_reset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}


