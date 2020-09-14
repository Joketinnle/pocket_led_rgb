#include "includes.h"
#include "led.h"

/**
 * RGB LED
 * PB6 => blue TIM4_CH1
 * PB7 => green TIM4_CH2
 * PB8 => red TIM4_CH3
 * 
 * C&W LED
 * PA6 => warm TIM3_CH1
 * PA7 => cold TIM3_CH2
 * 
 * pwm value max: 255
 * 
*/

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

void led_init(void)
{
    uint8_t duty = 0;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty); //w
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty); //c
    
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, duty);//b
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, duty);//g
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, duty);//r

}

void led_output_start(void)
{
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
}

void led_output_stop(void)
{
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_3);
}

void led_output_value(led_bright_t *led)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, led->w);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, led->c);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, led->b);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, led->g);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, led->r);
}

static void led_stop_rgb(void)
{
    HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_3);
}
static void led_start_rgb(void)
{
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
}

static void led_stop_cw(void)
{
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_2);
}

static void led_start_cw(void)
{
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
}

void led_rgb_update(uint16_t hue, uint8_t sat, uint8_t val)
{
    static hsv_t hsv;
    static led_bright_t led;

    hsv.h = hue;
    hsv.s = sat;
    hsv.v = val;

    led_stop_cw();


    hsv2rgb(&hsv, &led);
    led_output_value(&led);

    led_start_rgb();
}


void led_cw_update(uint16_t color_tmp, uint8_t color_bright)
{
    static led_bright_t led;
    float color_tmp_offset;
    float cold_sector;
    float cold_pct;
    float warm_pct;
    float bright_pct;

    led_stop_rgb();

    if (color_tmp > COLOR_TMP_COLD_MAX || color_tmp < COLOR_TMP_WARM_MAX)
        return;
    
    color_tmp_offset = (float)(color_tmp - COLOR_TMP_WARM_MAX);
    cold_sector = COLOR_TMP_COLD_MAX - COLOR_TMP_WARM_MAX;

    if (color_tmp_offset != 0)
        cold_pct = (float)(color_tmp_offset/cold_sector);
    else 
        cold_pct = 0.0f;

    warm_pct = (float)(1.0f - cold_pct);
    bright_pct = (float)(color_bright/100.0f);
    led.c = (uint8_t)(cold_pct * bright_pct * 255);
    led.w = (uint8_t)(warm_pct * bright_pct * 255);

    // printf("color_tmp: %d\r\n", color_tmp);
    // printf("cold_pct %f, warm_pct %f\r\n", cold_pct, warm_pct);
    // printf("c: %d, w: %d\r\n",led.c,led.w);
    led_output_value(&led);
    led_start_cw();
}

static void led_scen_police(void)
{
    static uint16_t tmp = 0;
    led_bright_t led_val;
    led_val.r = 0;
    led_val.g = 0;
    led_val.b = 0;
    led_val.c = 0;
    led_val.w = 0;

    if (tmp < 12) {
        if (tmp%2)
            led_val.r = 255;
        led_output_value(&led_val);
        osDelay(50);
    } else if (tmp > 12 && tmp <24) {
        if (tmp%2) 
            led_val.b = 255;
        led_output_value(&led_val);
        osDelay(50);
    } else if (tmp == 12) {
        led_output_value(&led_val);
        osDelay(200);
    } else if (tmp == 24) {
        led_output_value(&led_val);
        osDelay(200);
        tmp = 0;
    }

    if (tmp < 24)
        tmp++;
}

static void led_scen_ambulience(void)
{
    static uint16_t tmp = 0;
    static uint8_t red = 0;
    static uint8_t blue = 0;
    led_bright_t led_val;
    led_val.r = 0;
    led_val.g = 0;
    led_val.b = 0;
    led_val.c = 0;
    led_val.w = 0;

    if (tmp < 256) {
        if (tmp == 0)
            red = 0;
        else
            red++;
    } else if (tmp >= 256 && tmp < 512) {
        red--;
        if (tmp == 511)
            red = 0;
    } else if (tmp >= 512 && tmp < 768) {
        if (tmp == 512)
            blue = 0;
        else 
            blue++;
    } else if (tmp >= 768 && tmp < 1024) {
        blue--;
        if (tmp == 1023)
            blue = 0;
    }
    tmp++;
    if (tmp > 1023)
        tmp = 0;
    led_val.r = red;
    led_val.b = blue;
    led_output_value(&led_val);
    osDelay(1);

}

static void led_scen_lightning(void)
{
    static uint16_t tmp = 0;
    // static uint8_t light_type = 0;
    static int cold_color = 0;
    led_bright_t led_val;
    led_val.r = 0;
    led_val.g = 0;
    led_val.b = 0;
    led_val.c = 0;
    led_val.w = 0;

    if (tmp < 10) {
        if (tmp == 0)
            cold_color = 0;
        else 
            cold_color += 25;            
    } else if (tmp == 10) {
        osDelay(50);
    } else if (tmp < 16 && tmp > 10) {
        cold_color -= 50;
        if (tmp == 15)
            cold_color = 0;
    } else if (tmp < 21) {
        cold_color += 50;
    } else if (tmp == 21) {
        osDelay(100);
    } else if (tmp > 21 && tmp <= 31) {
         cold_color -= 25;
         if (tmp == 31)
            cold_color = 0;
    } else if (tmp >= 32 && tmp<= 42) {
        osDelay(100);
    }
    tmp++;
    if (tmp >= 42) 
        tmp = 0;
    led_val.c = (uint8_t)cold_color;
    led_output_value(&led_val);
    osDelay(10);
}

void led_scen_fire(void)
{
    static uint16_t tmp = 0;
    static led_bright_t led;
    static hsv_t hsv;
    static int val = 100;
    static uint8_t cnt = 0;

    hsv.h = 5;
    hsv.s = 100;
    
    if(tmp == 0)
        val = 100;
    
    if (tmp < 20) {
        val -= 3;
    } else if (tmp >= 20 && tmp < 40) {
        val += 3;
        if (tmp == 39 && cnt < 1) {
            cnt++;
            tmp = 0;
            val = 100;
        }
    } 
    else if (tmp >= 40 && tmp < 42) {
        val -= 30;
    } else if (tmp >= 42 && tmp < 44) {
        val += 30;
    } else if (tmp >= 44 && tmp < 46) {
        val -= 30;
    } else if (tmp >= 46 && tmp < 48) {
        val += 30;
    } else if (tmp >= 48 && tmp < 50) {
        val -= 30;
    } else if (tmp >= 50 && tmp < 52) {
        val += 30;
    } else if (tmp == 52){
        tmp = 0;
        cnt = 0;
    }

    tmp++;
    if (val > 100)
        val = 100;
    else if (val < 0)
        val = 0;
    hsv.v = (uint8_t)val;
    hsv2rgb(&hsv, &led);
    led_output_value(&led);
    if (tmp<40)
        osDelay(100);    
    else if (tmp > 40 && (tmp-40)%4)
        osDelay(20);
    else if (tmp > 40 && !((tmp-40)%4))
        osDelay(100);
}

void led_scen(enum SCENES_SELECT SCENES)
{
    switch (SCENES) {
    case NONE:
        led_output_stop();
        break;
    case POLICE:
        led_output_start();
        led_scen_police();
        break;

    case AMBULIENCE:
        led_output_start();
        led_scen_ambulience();
        break;
    
    case LIGHTNING:
        led_output_start();
        led_scen_lightning();
        break;

    case FIRE:
        led_output_start();
        led_scen_fire();
        break;
    
    case COLORCHASE:

        break;

    default:
        led_output_stop();
        break;
    }
}


