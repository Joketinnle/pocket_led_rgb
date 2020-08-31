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


