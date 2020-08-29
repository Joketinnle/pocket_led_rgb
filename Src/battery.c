#include "includes.h"
#include "battery.h"

#define BATTERY_0                   2.5f /* 2.5V */
#define BATTERY_20                  3.3f
#define BATTERY_40                  3.4f
#define BATTERY_60                  3.6f
#define BATTERY_80                  3.8f
#define BATTERY_100                 3.9f
/**
 * this file is for read battery val and tmp adc val
 * charging and charging complete.
 */


extern ADC_HandleTypeDef hadc1;

void battery_read_test(void)
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

    printf("adc ch1 val: %f\r\n", adc_val[0]);
    printf("adc ch2 val: %f\r\n", adc_val[1]);
}

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

    volt->battery = adc_val[0];
    volt->temp = adc_val[1];
    // printf("adc ch1 val: %f\r\n", adc_val[0]);
    // printf("adc ch2 val: %f\r\n", adc_val[1]);
}

static enum BATTERY_STATUS battery_adc_to_pct(float bat_volt)
{
    float bat_volt_tmp = 0.0;

    bat_volt_tmp = bat_volt*2;
    if (bat_volt_tmp >= BATTERY_100) 
        return REMI_100;
    else if (bat_volt_tmp >= BATTERY_60 && bat_volt_tmp < BATTERY_80)
        return REMI_80;
    else if (bat_volt_tmp >= BATTERY_40 && bat_volt_tmp < BATTERY_60)
        return REMI_60;
    else if (bat_volt_tmp >= BATTERY_20 && bat_volt_tmp < BATTERY_40)
        return REMI_40;
    else if (bat_volt_tmp >= BATTERY_0 && bat_volt_tmp < BATTERY_20)
        return REMI_20;
    else
        return REMI_0;
}

static bool battery_charging(void)
{
    if (HAL_GPIO_ReadPin(STAT_2_GPIO_Port, STAT_2_Pin) == GPIO_PIN_SET)
        return true;
    else 
        return false;
}

static bool battery_charging_complete(void)
{
    if (HAL_GPIO_ReadPin(STAT_1_GPIO_Port, STAT_1_Pin) == GPIO_PIN_SET)
        return true;
    else 
        return false;
}



void bettery_status_changed(void)
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
    // printf("bat charging: %d\r\n", bat_stat->chrg);
    // printf("bat chrg cmplt: %d\r\n", bat_stat->chrg_cmplt);
    // printf("bat val: %d\r\n",bat_stat->bat_pct);
    
    page->charging = bat_stat->chrg;

    if (bat_stat->chrg_cmplt == true)
        page->BAT = REMI_100;
    else 
        page->BAT = bat_stat->bat_pct;
}



/*

for(i=0;i<2;i++)
      {
          HAL_ADC_Start(&hadc1);
          HAL_ADC_PollForConversion(&hadc1,0xffff);
          ADC_Value[i]=HAL_ADC_GetValue(&hadc1);
      }
      HAL_ADC_Stop(&hadc1);
      ad1 = (float)(ADC_Value[0]&0xFFF)*3.3/4096;
      ad2 = (float)(ADC_Value[1]&0xFFF)*3.3/4096;

*/
