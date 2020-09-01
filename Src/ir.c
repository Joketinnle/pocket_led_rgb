#include "includes.h"
#include "ir.h"



/**
 * ir.c 
 * this file is for reading ir cmd that come from remote
 * NEC-code and 21 buttons
 * @todo ir_recv gpio should use TIM2 CH1 or CH2 port.
 * So that it can use Input capture to receive ir data;
 * <.......... button map ............>
 *      CH-     CH      Ch+             
 *      ⏮     ⏭      ⏯
 *      -       +       EQ
 *      0       100+    200+
 *      1       2       3
 *      4       5       6
 *      7       8       9
 * 
 * <.......... discribtion ............>
 * CH- : [RGB] Hue-          [C&W] None
 * CH  :
 * CH+ : [RGB] Hue+          [C&W] None
 * ⏮ : [RGB] Saturation-    [C&W] Color_temp-
 * ⏭ : [RGB] Saturation+    [C&W] Color_temp+
 * ⏯ : [RGB][C&W] on/off
 *  -  : [RGB] Value-        [C&W] Bright-
 *  +  : [RGB] Value+        [C&W] Bright+
 * EQ  :
 * 100+:
 * 200+:
 * 0~9 : Scene 0~9
 * https://github.com/albertyeh/STM32_NEC_Decode
*/


extern TIM_HandleTypeDef htim2;

static bool g_ir_read_start = true;
static uint32_t g_ir_recv_cnt = 0;
static enum NEC_CODE_STATUE IR_STA = HEADER;



static void ir_read_data(void)
{
    uint32_t cnter;
    cnter = __HAL_TIM_GET_COUNTER(&htim2);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start(&htim2);
    osTimerStart(irTimerHandle, IR_TIMEOUT_MS);    
    
    switch (IR_STA) {
    case HEADER:
        if (g_ir_recv_cnt == 0) {
            /* first time, do nothing */
        } else if (g_ir_recv_cnt == 1) {
            if (cnter >= (HEADER_HIGH+ERROR_DIFF) || cnter <=  (HEADER_HIGH-ERROR_DIFF))
                IR_STA = ABANDON;
            else
                IR_STA = HEADER;
        } else if (g_ir_recv_cnt == 2) {
            if (cnter >= (HEADER_LOW+ERROR_DIFF) || cnter <=  (HEADER_LOW-ERROR_DIFF))
                IR_STA = ABANDON;
            else 
                IR_STA = ADDR;
        }
        
        break;

    case ADDR:

        break;

    case CMD:

        break;

    case REPEAT:

        break;
    case ABANDON:

        break;
    }

    g_ir_recv_cnt++;
    /* first packet init */
    if (true == g_ir_read_start) {
        g_ir_read_start = false;
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        HAL_TIM_Base_Start(&htim2);
        osTimerStart(irTimerHandle, IR_TIMEOUT_MS);
        g_ir_recv_cnt = 0;
    } else {
        cnt_tmp = __HAL_TIM_GET_COUNTER(&htim2);
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        HAL_TIM_Base_Start(&htim2);
        osTimerStart(irTimerHandle, IR_TIMEOUT_MS);
        g_ir_recv_cnt++;
        printf("%d\r\n", cnt_tmp);
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
    printf("done cnt = %d\r\n\r\n", ir_recv_cnt);
    osTimerStop(irTimerHandle);
}
