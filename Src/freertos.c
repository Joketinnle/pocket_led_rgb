/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2020 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "includes.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osMessageQId batteryQueueHandle;
osMessageQId ircmdQueueHandle;
osMessageQId secnQueueHandle;
osMailQId batteryMailHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId secnTaskHandle;
osThreadId dispTaskHandle;
osMessageQId irQueueHandle;
osMessageQId keyQueueHandle;
osTimerId keyTimerHandle;
osTimerId irTimerHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void secn_task(void const * argument);
void dispaly_task(void const * argument);
void key_timer_callback(void const * argument);
void ir_timer_callback(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of keyTimer */
  osTimerDef(keyTimer, key_timer_callback);
  keyTimerHandle = osTimerCreate(osTimer(keyTimer), osTimerPeriodic, NULL);

  /* definition and creation of irTimer */
  osTimerDef(irTimer, ir_timer_callback);
  irTimerHandle = osTimerCreate(osTimer(irTimer), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 64);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of secnTask */
  osThreadDef(secnTask, secn_task, osPriorityIdle, 0, 128);
  secnTaskHandle = osThreadCreate(osThread(secnTask), NULL);

  /* definition and creation of dispTask */
  osThreadDef(dispTask, dispaly_task, osPriorityHigh, 0, 256);
  dispTaskHandle = osThreadCreate(osThread(dispTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of irQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(irQueue, 64, uint16_t);
  irQueueHandle = osMessageCreate(osMessageQ(irQueue), NULL);

  /* definition and creation of keyQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(keyQueue, 16, uint32_t);
  keyQueueHandle = osMessageCreate(osMessageQ(keyQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */

  osMessageQDef(batteryQueue, 8, uint8_t);
  batteryQueueHandle = osMessageCreate(osMessageQ(batteryQueue), NULL);

  osMessageQDef(ircmdQueue, 32, uint8_t);
  ircmdQueueHandle = osMessageCreate(osMessageQ(ircmdQueue), NULL);

  osMessageQDef(secnQueue, 8, enum SCENES_SELECT);
  secnQueueHandle = osMessageCreate(osMessageQ(secnQueue), NULL);

  osMailQDef(batteryMail, 2, struct batter_status);
  batteryMailHandle = osMailCreate(osMailQ(batteryMail), NULL);

  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  sys_show_info();
  /* Infinite loop */
  for(;;)
  {
      bettery_status_check();
      osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_secn_task */
/**
* @brief Function implementing the secnTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_secn_task */
void secn_task(void const * argument)
{
  /* USER CODE BEGIN secn_task */
  osEvent msg;
  enum SCENES_SELECT SCENES;
  bool scenes_off = false;
  /* Infinite loop */
  for(;;)
  {
    msg = osMessageGet(secnQueueHandle, 6);
    if (msg.status == osEventMessage) {
        SCENES = msg.value.v;
    }
    if (SCENES != NONE) {
        // led_scen(SCENES);
        scenes_off = false;
    } else {
        if (scenes_off == false) {
            scenes_off = true;
            led_scen(SCENES);
      } 
    }
    osDelay(1);
  }
  /* USER CODE END secn_task */
}

/* USER CODE BEGIN Header_dispaly_task */
/**
* @brief Function implementing the dispTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_dispaly_task */
void dispaly_task(void const * argument)
{
  /* USER CODE BEGIN dispaly_task */
  osEvent msg;
  uint8_t key_p_m_long_sta = 0;
  uint32_t delay_tmp = 0;
  const uint8_t long_press_delay = 10; /* ms */
  static struct page_info pg_info;
  struct batter_status *bat_stat;
  osTimerStart(keyTimerHandle, KEY_DELAY_MS);
  ir_recv_deinit();
  ssd1306_init();
  disp_led_value_init(&pg_info);
  disp_update(&pg_info);

  /* Infinite loop */
  for(;;)
  {
    msg = osMessageGet(keyQueueHandle, 0);
    if (msg.status == osEventMessage) {
        key_p_m_long_sta = key_process(&pg_info, msg.value.v);
        disp_update(&pg_info);
    }

    msg = osMailGet(batteryMailHandle, 0);
    if (msg.status == osEventMail) {
        bat_stat = msg.value.p;
        battery_process(&pg_info, bat_stat);
        disp_update(&pg_info);
        osMailFree(batteryMailHandle, bat_stat);
    }

    /* ir cmd */
    msg = osMessageGet(ircmdQueueHandle, 0);
    if (msg.status == osEventMessage) {
        printf("ir cmd: %#x\r\n", msg.value.v);
        ir_cmd_process(msg.value.v, &pg_info);
        disp_update(&pg_info);
    }

    /* deal with key long press(plus and minus button) */
    if ((delay_tmp++)%long_press_delay == 0 && key_p_m_long_sta != 0) {
        if (key_p_m_long_sta == KEY_PLUS_LONG_PRESS) {
            key_process(&pg_info, KEY_SHORT_PRESS<<(KEY_PLUS*4));
        } else if (key_p_m_long_sta == KEY_MINUS_LONG_PRESS) {
            key_process(&pg_info, KEY_SHORT_PRESS<<(KEY_MINUS*4));
        }
        disp_update(&pg_info);
    }

    

    osDelay(1);
  }
  /* USER CODE END dispaly_task */
}

/* key_timer_callback function */
void key_timer_callback(void const * argument)
{
  /* USER CODE BEGIN key_timer_callback */
  key_poll();
  /* USER CODE END key_timer_callback */
}

/* ir_timer_callback function */
void ir_timer_callback(void const * argument)
{
  /* USER CODE BEGIN ir_timer_callback */
  ir_timer_callback_func();
  /* USER CODE END ir_timer_callback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
