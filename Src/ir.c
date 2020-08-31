#include "includes.h"
#include "ir.h"

extern TIM_HandleTypeDef htim2;

static bool g_ir_read_start = true;


/*
5059
526
662
665
669
663
667
663
667
1737
1601
1612
1606
1608
1613
1605
1610
1613
530
1739
529
663
664
1746
528
659
1739
528
1738
1602
1603
532
1741
*/



static void ir_read_data(void)
{
    if (true == g_ir_read_start) {
        g_ir_read_start = false;
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        HAL_TIM_Base_Start(&htim2);
        osTimerStart(irTimerHandle, IR_TIMEOUR_MS);
    } else {
        printf("%d\r\n",__HAL_TIM_GET_COUNTER(&htim2));
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        HAL_TIM_Base_Start(&htim2);
        osTimerStart(irTimerHandle, IR_TIMEOUR_MS);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (IR_Pin == GPIO_Pin) {
        ir_read_data();
    }
}

void ir_timer_callback_func(void)
{
    g_ir_read_start = true;
    printf("done\r\n\r\n");
    osTimerStop(irTimerHandle);
}
