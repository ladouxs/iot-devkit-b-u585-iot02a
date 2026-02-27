/**
  ******************************************************************************
  * @file    tim_callback.c
  * @author  Sylvain LADOUX
  * @brief   Main program body
  ******************************************************************************
**/

#include "main.h"
#include "app_threadx.h"
#include "stm32u5xx_ll_tim.h"

extern TX_EVENT_FLAGS_GROUP tx_led_event;

// ISR TIM2
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;    // Clear flag
        tx_event_flags_set(&tx_led_event, 0x1, TX_OR);
    }
}
