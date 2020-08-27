#include "includes.h"
#include "battery.h"

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