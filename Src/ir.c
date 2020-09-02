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
 * CH  : switch RGB and C&W mode
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
 * The owner of this website is @strawmanbobi he is a great coder.
 * And he is so kind and full of open source spirit.
 * 
*/
#define DEBUG_IR                       0

#if DEBUG_IR
#define deubg_ir(...) printf(__VA_ARGS__)
#else
#define deubg_ir(...)
#endif


extern TIM_HandleTypeDef htim2;

static enum NEC_CODE_STATUE IR_STA = HEADER;
static struct ir_data ir_dat;

static uint8_t g_ir_raw_data_cnt = 0;
static bool g_ir_read_start = true;
static uint32_t g_ir_recv_cnt = 0;
static uint32_t g_ir_raw_data[64];
static uint8_t ir_button_array[21] = {
    IR_CH_MINUS,    IR_CH_ENTER,    IR_CH_PLUS, 
    IR_PREV,        IR_NEXT,        IR_ONOFF,
    IR_VOL_MINUS,   IR_VOL_PLUS,    IR_EQ,
    IR_ZERO,        IR_100_PLUS,    IR_200_PLUS,
    IR_ONE,         IR_TWO,         IR_THREE,
    IR_FOUR,        IR_FIVE,        IR_SIX,
    IR_SEVEN,       IR_EIGHT,       IR_NINE
};


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
    for (i=0; i<16; i+=2) {
        if  ((buf[i] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->addr |= 0<<(i/2);
        } else if ((buf[i] <= (DATA_1_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_1_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_1_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_1_LOW-ERROR_DIFF))) {
            data->addr |= 1<<(i/2);
        } else {
            deubg_ir("ir addr recv error\r\n");
        }
    }
    for (i=0; i<16; i+=2) {
        if  ((buf[i+16] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->addr_inverse |= 0<<(i/2);
        } else if ((buf[i+16] <= (DATA_1_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_1_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_1_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_1_LOW-ERROR_DIFF))) {
            data->addr_inverse |= 1<<(i/2);
        } else {
            deubg_ir("ir addr_inverse recv error %d %d %d\r\n",i+16, buf[i+16], buf[i+17]);
        }
    }
    // deubg_ir("addr: %#x, addr inverse: %#x", data->addr, data->addr_inverse);
}

static void ir_cmd_check(uint32_t *buf, struct ir_data *data)
{
    uint8_t i;
    for (i=0; i<16; i+=2) {
        if  ((buf[i] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->cmd |= 0<<(i/2);
        } else if ((buf[i] <= (DATA_1_HIGH+ERROR_DIFF) &&  buf[i] >= (DATA_1_HIGH-ERROR_DIFF)) 
            && (buf[i+1] <= (DATA_1_LOW+ERROR_DIFF) &&  buf[i+1] >= (DATA_1_LOW-ERROR_DIFF))) {
            data->cmd |= 1<<(i/2);
        } else {
            deubg_ir("ir cmd recv error\r\n");
        }
    }
    for (i=0; i<16; i+=2) {
        if  ((buf[i+16] <= (DATA_0_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_0_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_0_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_0_LOW-ERROR_DIFF))) {
            data->cmd_inverse |= 0<<(i/2);
        } else if ((buf[i+16] <= (DATA_1_HIGH+ERROR_DIFF) &&  buf[i+16] >= (DATA_1_HIGH-ERROR_DIFF)) 
            && (buf[i+17] <= (DATA_1_LOW+ERROR_DIFF) &&  buf[i+17] >= (DATA_1_LOW-ERROR_DIFF))) {
            data->cmd_inverse |= 1<<(i/2);
        } else {
            deubg_ir("ir cmd_inverse recv error\r\n");
        }
    }
}

static ErrorStatus ir_cmd_process(struct ir_data *data)
{
    if ((data->addr ^ data->addr_inverse) != 0xFF) {
        deubg_ir("addr check failed %#x %#x\r\n", data->addr, data->addr_inverse);
        return ERROR;
    }
    if ((data->cmd ^ data->cmd_inverse) != 0xFF) {
        deubg_ir("cmd check failed %#x %#x\r\n", data->cmd, data->cmd_inverse);
        return ERROR;
    }
    deubg_ir("addr %#x cmd %#X\r\n",data->addr, data->cmd);
    
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
        deubg_ir("%d\r\n", cnt_tmp);
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
                g_ir_recv_cnt = 0;
            }
        }
        break;

    case ADDR:
        if (g_ir_recv_cnt >= 1 && g_ir_recv_cnt <= 32) {
            g_ir_raw_data[g_ir_raw_data_cnt++] = cnter;

            if (g_ir_recv_cnt == 32) {
                ir_addr_check(g_ir_raw_data, &ir_dat);
                g_ir_raw_data_cnt = 0;
                g_ir_recv_cnt = 0;
                IR_STA = CMD;
            }
        }
        break;

    case CMD:
        if (g_ir_recv_cnt >= 1 && g_ir_recv_cnt <= 32) {
            g_ir_raw_data[g_ir_raw_data_cnt++] = cnter;

            if (g_ir_recv_cnt == 32) {
                ir_cmd_check(g_ir_raw_data, &ir_dat);
                if (SUCCESS == ir_cmd_process(&ir_dat)) {
                    osMessagePut(ircmdQueueHandle, ir_dat.cmd, 0);
                    g_ir_raw_data_cnt = 0;
                    g_ir_recv_cnt = 0;
                    IR_STA = REPEAT;
                } else {
                    IR_STA = ABANDON;
                }
            }
        }
        break;

    case REPEAT:
        if (cnter < REPEAT_LOW+ERROR_DIFF && cnter > REPEAT_LOW-ERROR_DIFF) {
            if (ir_dat.cmd != 0x00)
                osMessagePut(ircmdQueueHandle, ir_dat.cmd, 0);
        }
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
    g_ir_recv_cnt = 0;
    IR_STA = HEADER;
    ir_data_init(&ir_dat);
    deubg_ir("done\r\n\r\n");
    osTimerStop(irTimerHandle);
}
