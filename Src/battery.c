#include "includes.h"
#include "battery.h"

#define BATTERY_0                   250 /* 2.5V * 100 = 250 */
#define BATTERY_20                  330
#define BATTERY_40                  340
#define BATTERY_60                  360
#define BATTERY_80                  380
#define BATTERY_100                 410
/**
 * this file is for read battery val and tmp adc val
 * charging and charging complete.
 */


extern ADC_HandleTypeDef hadc1;

static void adc_read(struct voltage *volt)
{
    uint16_t adc_tmp[2];
    float adc_val[2];
    
    for (uint8_t i=0; i<2; i++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 0xffff);
        adc_tmp[i] = HAL_ADC_GetValue(&hadc1);
        
    }
    HAL_ADC_Stop(&hadc1);

    adc_val[0] = (float)(adc_tmp[0]&0x0fff)*3.3/4096;
    adc_val[1] = (float)(adc_tmp[1]&0x0fff)*3.3/4096;

    volt->battery = adc_val[1];
    volt->temp = adc_val[0];
    // printf("adc ch1 val: %f\r\n", adc_val[0]);
    // printf("adc ch2 val: %f\r\n", adc_val[1]);
}

static enum BATTERY_STATUS battery_adc_to_pct(float bat_volt)
{
    static float battery_prev_avr = 0;
    uint32_t battery_volt = 0;
    enum BATTERY_STATUS ret;
    static uint8_t cnt = 0;
    static bool init = true;

    battery_volt = (uint32_t)(bat_volt*2.0*100.0);
    if (true == init) {
        cnt++;
        battery_prev_avr += battery_volt;
        if (cnt == 10) {
            init = false;
            battery_prev_avr/=10.0f;
        }
    } else {
        battery_volt = (uint32_t)(battery_prev_avr*0.9f + (float)battery_volt*0.1f);
    }
    
    if (battery_volt >= BATTERY_100) 
        ret = REMI_100;
    else if (battery_volt >= BATTERY_80 && battery_volt < BATTERY_100)
        ret = REMI_80;
    else if (battery_volt >= BATTERY_60 && battery_volt < BATTERY_80)
        ret = REMI_60;
    else if (battery_volt >= BATTERY_40 && battery_volt < BATTERY_60)
        ret = REMI_40;
    else if (battery_volt >= BATTERY_20 && battery_volt < BATTERY_0)
        ret = REMI_20;
    else
        ret = REMI_0;

    return ret;
}

/**
 * Charge IC: LP4056HSPF 
 * Charging:    
 *                  STA2: HIGH
 *                  STA1: LOW
 * Charging Completed:
 *                  STA2: LOW
 *                  STA1: HIGH
 * No Charging:
 *                  STA2: HIGH
 *                  STA1: HIGH
*/
static bool battery_charging(void)
{
    if (HAL_GPIO_ReadPin(STAT_2_GPIO_Port, STAT_2_Pin) == GPIO_PIN_SET 
        && HAL_GPIO_ReadPin(STAT_1_GPIO_Port, STAT_1_Pin) == GPIO_PIN_RESET)
        return true;
    else 
        return false;
}

static bool battery_charging_complete(void)
{
    if (HAL_GPIO_ReadPin(STAT_2_GPIO_Port, STAT_2_Pin) == GPIO_PIN_RESET 
        && HAL_GPIO_ReadPin(STAT_1_GPIO_Port, STAT_1_Pin) == GPIO_PIN_SET)
        return true;
    else 
        return false;
}

void bettery_status_check(void)
{
    static struct batter_status bat_sta_prev;
    static struct batter_status bat_sta;
    struct batter_status *bat_tmp;
    struct voltage volt;
    bool flag = false;

    adc_read(&volt);
    bat_sta.chrg = battery_charging();
    bat_sta.bat_pct = battery_adc_to_pct(volt.battery);
    bat_sta.chrg_cmplt = battery_charging_complete();
    
    if (bat_sta_prev.chrg_cmplt != bat_sta.chrg_cmplt || 
            bat_sta_prev.chrg != bat_sta.chrg ||
            bat_sta_prev.bat_pct != bat_sta.bat_pct )
    {
        flag = true;
        bat_sta_prev.chrg = bat_sta.chrg;
        bat_sta_prev.bat_pct = bat_sta.bat_pct;
        bat_sta_prev.chrg_cmplt = bat_sta.chrg_cmplt;
    }

    if (true == flag) {
        bat_tmp = osMailAlloc(batteryMailHandle, osWaitForever);
        bat_tmp->chrg = bat_sta.chrg;
        bat_tmp->bat_pct = bat_sta.bat_pct;
        bat_tmp->chrg_cmplt = bat_sta.chrg_cmplt;
        osMailPut(batteryMailHandle, bat_tmp);
    }
}

void battery_process(struct page_info *page, struct batter_status *bat_stat)
{    
    page->charging = bat_stat->chrg;

    if (bat_stat->chrg_cmplt == true)
        page->BAT = REMI_100;
    else 
        page->BAT = bat_stat->bat_pct;
}
