/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  Sylvain LADOUX
  * @brief   Main program body
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"
#include "main.h"
#include "stm32u5xx_ll_tim.h"

// #include "b_u585i_iot02a.h"

/* Private includes ----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TX_THREAD tx_app_thread;
TX_THREAD tx_led_thread;
TX_EVENT_FLAGS_GROUP tx_led_event;
UCHAR led_thread_stack[1024];
UCHAR app_thread_stack[1024];
extern TIM_HandleTypeDef htim2;

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  /* USER CODE END App_ThreadX_MEM_POOL */
  CHAR *pointer;

  /* Allocate the stack for tx app thread  */
  if (tx_byte_allocate(byte_pool, (VOID**) &pointer,
                       TX_APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }
  /* Create tx app thread.  */
  if (tx_thread_create(&tx_app_thread, "tx app thread", tx_app_thread_entry, 0, pointer,
                       TX_APP_STACK_SIZE, TX_APP_THREAD_PRIO, TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE, TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  /* USER CODE BEGIN App_ThreadX_Init */
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

/**
  * @brief  Function implementing the tx_app_thread_entry thread.
  * @param  thread_input: Hardcoded to 0.
  * @retval None
  */
void tx_app_thread_entry(ULONG thread_input)
{
  (void)thread_input;

  while(1)
  {
    /* Some code */
  }
}

void tx_led_thread_entry(ULONG thread_input)
{
    (void)thread_input;
    ULONG flags;

    while (1)
    {
        // printf("[Debug trace] In tx_led_thread_entry loop (step 1)\n");

        tx_event_flags_get(&tx_led_event,
                           0x1,
                           TX_OR_CLEAR,
                           &flags,
                           TX_WAIT_FOREVER);

        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

      /**
       * Without IRQ
       * Technical note: In a ThreadX environment, HAL_Delay() is completely forgotten.
       */
      // HAL_GPIO_WritePin(GPIOH, LED_GREEN_Pin, GPIO_PIN_RESET); // ON
      // tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 20);

      // HAL_GPIO_WritePin(GPIOH, LED_GREEN_Pin, GPIO_PIN_SET);   // OFF
      // tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 20);

      // printf("[Debug trace] In tx_led_thread_entry loop (step 2)\n");
    }
}


  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  tx_kernel_enter();
}

void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;

    tx_event_flags_create(&tx_led_event, "LED EVENT");

    tx_thread_create(&tx_led_thread,
                  "Led Thread",
                  tx_led_thread_entry,
                  0,
                  led_thread_stack,
                  sizeof(led_thread_stack),
                  1,
                  1,
                  TX_NO_TIME_SLICE,
                  TX_AUTO_START);

  // __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
  // __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

  // HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
  // HAL_NVIC_EnableIRQ(TIM2_IRQn);

  // // LL_TIM_EnableCounter(TIM2);
  // HAL_TIM_Base_Start(&htim2);

  /* Start TIM2 with interrupt */
  HAL_TIM_Base_Start_IT(&htim2);
}

