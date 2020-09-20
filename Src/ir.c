#include "includes.h"
#include "ir.h"


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
#if defined(__IR__DEBUG__)  
static bool g_ir_read_start = true;
#endif
static uint32_t g_ir_recv_cnt = 0;
static uint32_t g_ir_raw_data[64];
// static uint8_t ir_button_array[21] = {
//     IR_CH_MINUS,    IR_CH_ENTER,    IR_CH_PLUS, 
//     IR_PREV,        IR_NEXT,        IR_ONOFF,
//     IR_VOL_MINUS,   IR_VOL_PLUS,    IR_EQ,
//     IR_ZERO,        IR_100_PLUS,    IR_200_PLUS,
//     IR_ONE,         IR_TWO,         IR_THREE,
//     IR_FOUR,        IR_FIVE,        IR_SIX,
//     IR_SEVEN,       IR_EIGHT,       IR_NINE
// };

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

static ErrorStatus ir_data_check(struct ir_data *data)
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

void ir_recv_enable(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOC_IS_CLK_ENABLED();
    GPIO_InitStruct.Pin = IR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IR_GPIO_Port, &GPIO_InitStruct);   
}


void ir_read_data(void)
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
                if (SUCCESS == ir_data_check(&ir_dat)) {
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



void ir_timer_callback_func(void)
{
#if defined(__IR__DEBUG__)
    g_ir_read_start = true;
#endif	
    g_ir_recv_cnt = 0;
    IR_STA = HEADER;
    ir_data_init(&ir_dat);
    deubg_ir("done\r\n\r\n");
    osTimerStop(irTimerHandle);
}

/**
 * ir_recv_init()
 * this func is called by key.c
 * when power key get a long press(power on)
 * turn on ir recv
*/
void ir_recv_init(void)
{
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void ir_recv_deinit(void)
{
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
}

void ir_cmd_process(uint8_t ir_cmd, struct page_info *page)
{
    switch (ir_cmd) {
    case IR_CH_MINUS:
        page->PAGE = PAGE_RGB;
        page->select_num = RGB_HUE;
        if (page->hue > 5)
            page->hue -= 5;
        else
            page->hue = 0;
        break;

    case IR_CH_ENTER:
        if (page->PAGE == PAGE_RGB) {
            page->PAGE = PAGE_CW;
            page->select_num = CW_TEMP;
        } else { 
            page->PAGE = PAGE_RGB;
            page->select_num = RGB_HUE;
        }
        break;

    case IR_CH_PLUS:
        page->PAGE = PAGE_RGB;
        if (page->hue < 355)
            page->hue += 5;
        else 
            page->hue = 360;
        break;

    case IR_PREV:
        if (page->PAGE == PAGE_RGB) {
            page->select_num = RGB_SAT;
            if (page->sat > 5)
                page->sat -= 5;
            else 
                page->sat = 0;
        } else if (page->PAGE == PAGE_CW) {
            page->select_num = CW_TEMP;
            if (page->color_temp > 3050)
                page->color_temp -= 50;
            else 
                page->color_temp = 3000;
        }
        break;

    case IR_NEXT:
        if (page->PAGE == PAGE_RGB) {
            page->select_num = RGB_SAT;
            if (page->sat < 95)
                page->sat += 5;
            else 
                page->sat = 100;
        } else if (page->PAGE == PAGE_CW) {
            page->select_num = CW_TEMP;
            if (page->color_temp < 6950)
                page->color_temp += 50;
            else 
                page->color_temp = 7000;
        }
        break;

    case IR_ONOFF:
        if (page->LED_SWITCH == ON) {
            page->LED_SWITCH = OFF;
            led_output_stop();
        } else if (page->LED_SWITCH == OFF) {
            page->LED_SWITCH = ON;
        }
        break;

    case IR_VOL_MINUS:
        if (page->PAGE == PAGE_RGB) {
            page->select_num = RGB_VAL;
            if (page->val > 5)
                page->val -= 5;
            else 
                page->val = 0;
        } else if (page->PAGE == PAGE_CW) {
            page->select_num = CW_BRIGHT;
            if (page->brightness > 5)
                page->brightness -= 5;
            else 
                page->brightness = 0;
        }
        break;

    case IR_VOL_PLUS:
        if (page->PAGE == PAGE_RGB) {
            page->select_num = RGB_VAL;
            if (page->val <95 )
                page->val += 5;
            else 
                page->val = 100;
        } else if (page->PAGE == PAGE_CW) {
            page->select_num = CW_BRIGHT;
            if (page->brightness < 95)
                page->brightness += 5;
            else 
                page->brightness = 100;
        }
        break;
    
    case IR_EQ:

        break;

    case IR_ZERO:

        break;

    case IR_100_PLUS:

        break;

    case IR_200_PLUS:

        break;

    case IR_ONE:

        break;

    case IR_TWO:

        break;

    case IR_THREE:

        break;

    case IR_FOUR:

        break;

    case IR_FIVE:

        break;

    case IR_SIX:

        break;

    case IR_SEVEN:

        break;

    case IR_EIGHT:

        break;

    case IR_NINE:

        break;
    }
}
