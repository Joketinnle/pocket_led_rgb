#include "includes.h"



#define KEY_NUM_MAX                     4

#define KEY_PRESS_SHORT_PERIOD                  (5*KEY_DELAY_MS)  /* 100ms */
#define KEY_PRESS_LONG_PERIOD                   (50*KEY_DELAY_MS) /* 1000ms */


#define DEBUG_KEY                       0

#if DEBUG_KEY
#define deubg_k(...) printf(__VA_ARGS__)
#else
#define deubg_k(...)
#endif


extern osMessageQId keyQueueHandle;

static uint16_t s_key_press_time[KEY_NUM_MAX];
static uint16_t s_key_release_time[KEY_NUM_MAX];

static GPIO_TypeDef* GPIO_PORT_KEY[KEY_NUM_MAX] = {
    POWER_GPIO_Port, MENU_GPIO_Port, PLUS_GPIO_Port, MINUS_GPIO_Port
};

static uint16_t GPIO_PIN_KEY[KEY_NUM_MAX] = {
    POWER_Pin, MENU_Pin, PLUS_Pin, MINUS_Pin
};

static uint16_t key_get_status(void)
{
    uint16_t key_sta = 0;
    
    for (uint8_t i=0; i<KEY_NUM_MAX; i++) {
        if (GPIO_PIN_SET != HAL_GPIO_ReadPin(GPIO_PORT_KEY[i], GPIO_PIN_KEY[i])) {
            key_sta |= 1 << i;
        }
    }
    return key_sta;
}

static uint8_t key_get_change(uint16_t key_sta, uint8_t keynum)
{
    uint8_t ret_sta = KEY_NONE_ACTION;
    
    if (key_sta & (1<<keynum)) {
        /* key pressed */
        if (s_key_press_time[keynum] < 0xffff)
            s_key_press_time[keynum]++;

        if (s_key_press_time[keynum] == KEY_PRESS_SHORT_PERIOD/KEY_DELAY_MS)
            ret_sta = KEY_SHORT_PRESS;
        else if (s_key_press_time[keynum] == KEY_PRESS_LONG_PERIOD/KEY_DELAY_MS)
            ret_sta = KEY_LONG_PRESS;

        s_key_release_time[keynum] = 0;
    } else {
        /* key release */ 
        if(s_key_release_time[keynum] < 0xFFFF)
			s_key_release_time[keynum]++;

        if ((s_key_press_time[keynum] >= KEY_PRESS_SHORT_PERIOD/KEY_DELAY_MS)
                 && (s_key_press_time[keynum] < KEY_PRESS_LONG_PERIOD/KEY_DELAY_MS))
            ret_sta = KEY_SHORT_RELEASE;
        else if (s_key_press_time[keynum] >= KEY_PRESS_LONG_PERIOD/KEY_DELAY_MS)
            ret_sta = KEY_LONG_RELEASE;
        
        s_key_press_time[keynum] = 0;
    }
    return ret_sta;
}

/**
 * s_key_sta_update data format
 * each button use 4 bits
 * such as button 0 and 1 were pressed 0x0000 0011
 * short press 0b01  short release 0b10
 * long press 0b11   long release 0b100
*/
void key_poll(void)
{
    static uint32_t s_key_sta_update = 0;
    uint16_t key_sta_tmp = 0; 
    uint8_t key_sta = 0;
    
    s_key_sta_update = 0;

    key_sta_tmp = key_get_status();
    
    for (uint8_t i=0; i<KEY_NUM_MAX; i++) {
        key_sta = key_get_change(key_sta_tmp, i);
        if (key_sta == KEY_NONE_ACTION) 
            continue;
        deubg_k("button %d: sta: %d\r\n", i, key_sta);
        s_key_sta_update |= (uint32_t)(key_sta<<(i*4));
    }

    if (s_key_sta_update > 0)
        osMessagePut(keyQueueHandle, s_key_sta_update, 0);
}



uint8_t key_process(struct page_info *page, uint32_t key_sta)
{
    enum KEY_TYPE type;
    uint8_t key_act;
    uint8_t i;//KEY_NUM_MAX
    uint8_t ret = KEY_NONE_ACTION;
    for (i=0; i<KEY_NUM_MAX; i++) {
        type = i;
        key_act = (key_sta>>(i*4)) & 0x0000000F;

        if (page->PAGE==PAGE_OFF && type!=KEY_POWER)
            continue;

        switch (type) {
        case KEY_POWER:
            if (key_act == KEY_SHORT_RELEASE) {
                if (page->PAGE == PAGE_OFF)
                    break;
                if (page->select_num >= 1)
                    page->select_num--;
            } else if (key_act == KEY_LONG_PRESS) {
                if (page->PAGE == PAGE_OFF) {
                    page->PAGE = PAGE_RGB;//PAGE_OFF;//PAGE_SCENES;//PAGE_CW;//PAGE_RGB;
                    page->select_num = RGB_HUE;
                    page->hue = 0;
                    page->sat = 0;
                    page->val = 0;
                    page->color_temp = 3000;
                    page->brightness = 0;
                    page->SECN = NONE;
                    /* @debug */
                    page->charging = true;
                    page->BAT = REMI_100;
                } else {
                    page->PAGE = PAGE_OFF;
                }
            }
            break;

        case KEY_MENU:
            if (key_act == KEY_SHORT_RELEASE) {
                if (page->select_num < SCENES_PG)
                    page->select_num++;
            }
            break;

        case KEY_PLUS:
            if (key_act == KEY_SHORT_PRESS) {
                switch (page->select_num) {
                case RGB_HUE:
                    if (++(page->hue) > 360)
                        page->hue = 360;
                    break;
                case RGB_SAT:
                    if (++(page->sat) > 100)
                        page->sat = 100;
                    break;
                case RGB_VAL:
                    if (++(page->val) > 100)
                        page->val = 100;
                    break;
                case CW_TEMP:
                    if ((page->color_temp+=50) > 7000)
                        page->color_temp = 7000;
                    break;
                case CW_BRIGHT:
                    if (++(page->brightness) > 100)
                        page->brightness = 100;
                    break;
                case SCENES_PG:
                    if(++(page->SECN) == SCN_LAST)
                        page->SECN = NONE;
                    break;
                }
            } else if (key_act == KEY_LONG_PRESS) {
                ret = KEY_PLUS_LONG_PRESS;
            } else if (key_act == KEY_LONG_RELEASE) {
                ret = KEY_PLUS_LONG_RELEASE;
            }
            break;
        
        case KEY_MINUS:
            if (key_act == KEY_SHORT_PRESS) {
                switch (page->select_num) {
                case RGB_HUE:
                    if ((page->hue--) == 0)
                        page->hue = 0;
                    break;
                case RGB_SAT:
                    if ((page->sat--) == 0)
                        page->sat = 0;
                    break;
                case RGB_VAL:
                    if ((page->val--) == 0)
                        page->val = 0;
                    break;
                case CW_TEMP:
                    if ((page->color_temp-=50) <= 3000)
                        page->color_temp = 3000;
                    break;
                case CW_BRIGHT:
                    if ((page->brightness--) == 0)
                        page->brightness = 0;
                    break;
                case SCENES_PG:
                    if(--(page->SECN) == SCN_FIRST)
                        page->SECN = COLORCHASE;
                    break;
                }
            } else if (key_act == KEY_LONG_PRESS) {
                ret = KEY_MINUS_LONG_PRESS;
            } else if (key_act == KEY_LONG_RELEASE) {
                ret = KEY_MINUS_LONG_RELEASE;
            }
            break;
        }
				
				if (page->PAGE == PAGE_OFF)
						continue;
        if (page->select_num < CW_TEMP)
            page->PAGE = PAGE_RGB;
        else if (page->select_num == CW_TEMP || page->select_num == CW_BRIGHT)\
            page->PAGE = PAGE_CW;
        else
            page->PAGE = PAGE_SCENES;
    }

    return ret;

}



/*
#define KEY_NONE_ACTION                 0
#define KEY_SHORT_PRESS                 1
#define KEY_SHORT_RELEASE               2
#define KEY_LONG_PRESS                  3
#define KEY_LONG_RELEASE                4
*/

