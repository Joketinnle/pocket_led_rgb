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
 * 
 * Wanna know more about NEC Coding vist this website 👇
 *                                          https://irext.net/doc/
 * This website's owner is @strawmanbobi he is a great coder.
 * And he is so kind and full of open source spirit.
 * 
*/


extern TIM_HandleTypeDef htim2;

static bool g_ir_read_start = true;
static uint32_t g_ir_recv_cnt = 0;
static enum NEC_CODE_STATUE IR_STA = HEADER;
static struct ir_data ir_dat;

static uint8_t g_ir_raw_data_cnt = 0;
static uint32_t g_ir_raw_data[64];

static void ir_data_init(struct ir_data *data)
{
    data->addr = 0x00;
    data->addr_inverse = 0x00;
    data->cmd = 0x00;
    data->cmd_inverse = 0x00;
}

static void ir_addr_check(uint32_t *buf, struct ir_data *data)
{
    uint8_t i;
    for (i=0; i<16 ;i+=2) {
        if  ((buf[i] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->addr |= 0<<(i/2);
        } else if ((buf[i] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->addr |= 1<<(i/2);
        } else {
            printf("ir addr recv error\r\n");
        }
    }
    for (i=0; i<16 ;i+=2) {
        if  ((buf[i+16] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->addr_inverse |= 0<<(i/2);
        } else if ((buf[i+16] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->addr_inverse |= 1<<(i/2);
        } else {
            printf("ir addr_inverse recv error\r\n");
        }
    }
}
static void ir_cmd_check(uint32_t *buf, struct ir_data *data)
{
    uint8_t i;
    for (i=0; i<16 ;i+=2) {
        if  ((buf[i] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->cmd |= 0<<(i/2);
        } else if ((buf[i] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->cmd |= 1<<(i/2);
        } else {
            printf("ir cmd recv error\r\n");
        }
    }
    for (i=0; i<16 ;i+=2) {
        if  ((buf[i+16] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->cmd_inverse |= 0<<(i/2);
        } else if ((buf[i+16] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->cmd_inverse |= 1<<(i/2);
        } else {
            printf("ir cmd_inverse recv error\r\n");
        }
    }
}


static ErrorStatus ir_cmd_process(struct ir_data *data)
{
    if ((data->addr ^ data->addr_inverse) != 0xFF)
        return ERROR;
    if ((data->cmd ^ data->cmd_inverse) != 0xFF)
        return ERROR;

    printf("cmd %#X\r\n", data->cmd);
    
    return SUCCESS;
}


static void ir_read_data(void)
{


    /* for debug */
    /* first packet init */
#if defined(__IR__DEBUG__)    
    uint32_t cnt_tmp;
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
#else
    uint32_t cnter;
    cnter = __HAL_TIM_GET_COUNTER(&htim2);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start(&htim2);
    osTimerStart(irTimerHandle, IR_TIMEOUT_MS);    
    
    g_ir_recv_cnt++;
    switch (IR_STA) {
    case HEADER:
        if (g_ir_recv_cnt == 1) {
            /* first time, do nothing but start timer*/
        } else if (g_ir_recv_cnt == 2) {
            if (cnter >= (HEADER_HIGH+ERROR_DIFF) || cnter <= (HEADER_HIGH-ERROR_DIFF))
                IR_STA = ABANDON;
            else
                IR_STA = HEADER;
        } else if (g_ir_recv_cnt == 3) {
            if (cnter >= (HEADER_LOW+ERROR_DIFF) || cnter <= (HEADER_LOW-ERROR_DIFF)) {
                IR_STA = ABANDON;
                g_ir_recv_cnt = 0;
                g_ir_raw_data_cnt = 0;
                ir_data_init(&ir_dat);
            } else {
                IR_STA = ADDR;
            }
        }
        break;

    case ADDR:
        if (g_ir_recv_cnt >= 1 && g_ir_recv_cnt <= 32) {
            g_ir_raw_data[g_ir_raw_data_cnt++] = cnter;
        } else {
            ir_addr_check(g_ir_raw_data, &ir_dat);
            g_ir_raw_data_cnt = 0;
            g_ir_recv_cnt = 0;
            IR_STA = CMD;
        }
        break;

    case CMD://ir_cmd_check
        if (g_ir_recv_cnt >= 1 && g_ir_recv_cnt <= 32) {
            g_ir_raw_data[g_ir_raw_data_cnt++] = cnter;
        } else { 
            ir_cmd_check(g_ir_raw_data, &ir_dat);
            if (SUCCESS == ir_cmd_process(&ir_dat)) {
                g_ir_raw_data_cnt = 0;
                g_ir_recv_cnt = 0;
                IR_STA = REPEAT;
            } else {
                IR_STA = ABANDON;
            }
        }
        break;

    case REPEAT:

        break;
    case ABANDON:
        ir_data_init(&ir_dat);
        g_ir_recv_cnt = 0;
        g_ir_raw_data_cnt = 0;
        IR_STA = HEADER;
        break;
    }

    


#endif

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
    IR_STA = HEADER;
    printf("done cnt = %d\r\n\r\n", ir_recv_cnt);
    osTimerStop(irTimerHandle);
}
