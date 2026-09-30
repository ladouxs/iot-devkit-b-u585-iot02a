/**
  ******************************************************************************
  * @file    stm32u5xx_hal_timebase_tim.c
  * @brief   HAL time base based on TIM6, instead of the default SysTick.
  *
  *          ThreadX's Cortex-M33/GNU port takes ownership of the SysTick
  *          exception once tx_kernel_enter() runs (see
  *          Core/Src/rtos/app_threadx.c / MX_ThreadX_Init()), so HAL can no
  *          longer rely on SysTick to keep HAL_GetTick() / HAL_Delay()
  *          working. This overrides HAL's weak HAL_InitTick() /
  *          HAL_SuspendTick() / HAL_ResumeTick() to use TIM6 instead -- the
  *          same pattern STM32CubeMX generates automatically for
  *          ThreadX/FreeRTOS projects. TIM6 was free.
  ******************************************************************************
  * @attention
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;

/**
  * @brief  This function configures TIM6 as the HAL time base source.
  *         The time source is configured to have 1ms time base with a
  *         dedicated tick interrupt priority.
  * @note   Called automatically at the beginning of the program after reset
  *         by HAL_Init(), or at any time when the clock is (re)configured by
  *         HAL_RCC_ClockConfig(). Overrides the __weak default implementation
  *         in stm32u5xx_hal.c, which uses SysTick -- SysTick belongs to
  *         ThreadX in this project.
  * @param  TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef clkconfig;
  uint32_t uwTimclock, uwAPB1Prescaler;
  uint32_t uwPrescalerValue;
  uint32_t pFLatency;
  HAL_StatusTypeDef status;

  /* Enable TIM6 clock */
  __HAL_RCC_TIM6_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* Get APB1 prescaler */
  uwAPB1Prescaler = clkconfig.APB1CLKDivider;

  /* Compute TIM6 clock */
  if (uwAPB1Prescaler == RCC_HCLK_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
  }

  /* Compute the prescaler value to have TIM6 counter clock equal to 1 MHz */
  uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

  /* Initialize TIM6 */
  htim6.Instance = TIM6;

  /* Initialize TIM6 peripheral as follows:
     + Period = [(TIM6CLK/1000) - 1] to have a 1ms (1/1000 s) time base.
     + Prescaler = (uwTimclock/1000000 - 1) to have a 1 MHz counter clock.
     + ClockDivision = 0
     + Counter direction = Up
  */
  htim6.Init.Period = (1000000U / 1000U) - 1U;
  htim6.Init.Prescaler = uwPrescalerValue;
  htim6.Init.ClockDivision = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  status = HAL_TIM_Base_Init(&htim6);
  if (status == HAL_OK)
  {
    /* Start the TIM time base generation in interrupt mode */
    status = HAL_TIM_Base_Start_IT(&htim6);
    if (status == HAL_OK)
    {
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        /* Enable the TIM6 global interrupt */
        HAL_NVIC_SetPriority(TIM6_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      }
      else
      {
        status = HAL_ERROR;
      }
    }
  }

  HAL_NVIC_EnableIRQ(TIM6_IRQn);

  return status;
}

/**
  * @brief  Suspends the tick increment.
  * @note   Overrides the __weak default implementation. Disables TIM6 update
  *         interrupt so HAL_IncTick() is not called.
  * @retval None
  */
void HAL_SuspendTick(void)
{
  __HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
}

/**
  * @brief  Resumes the tick increment.
  * @note   Overrides the __weak default implementation. Re-enables TIM6
  *         update interrupt so HAL_IncTick() is called again.
  * @retval None
  */
void HAL_ResumeTick(void)
{
  __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}

/**
  * @brief  Period elapsed callback in non-blocking mode.
  * @param  htim: TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}
