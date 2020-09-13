#ifndef __INCLUDES_H
#define __INCLUDES_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stdbool.h"
#include "stdarg.h"
#include "stdint.h"
#include "usart.h"
#include "stdio.h"
#include "task.h"
#include "main.h"
#include "math.h"

/* my includes */
#include "sysconfig.h"
#include "ssd_1306.h"
#include "display.h"
#include "hsv2rgb.h"
#include "battery.h"
#include "key.h"
#include "led.h"
#include "ir.h"

extern osMessageQId batteryQueueHandle;
extern osMessageQId ircmdQueueHandle;
extern osMessageQId secnQueueHandle;
extern osMailQId batteryMailHandle;

extern osTimerId irTimerHandle;

#endif
