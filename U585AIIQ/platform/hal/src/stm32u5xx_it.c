/**
  ******************************************************************************
  * @file    stm32u5xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32u5xx_hal.h"
#include <stdio.h>
#include "main.h"
#include "stm32u5xx_it.h"
#include "tx_api.h"
#include "app_threadx.h"
#include "mx_wifi_conf.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern TX_EVENT_FLAGS_GROUP tx_led_event;
extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef handle_GPDMA1_Channel5;
extern DMA_HandleTypeDef handle_GPDMA1_Channel4;
extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c3;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

void HardFault_Handler(void)
{
    __asm volatile
    (
        "TST lr, #4            \n"
        "ITE EQ                \n"
        "MRSEQ r0, MSP         \n"
        "MRSNE r0, PSP         \n"
        "B HardFault_HandlerC  \n"
    );
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_HandlerC(uint32_t *hardfault_args)
{
    volatile uint32_t stacked_r0  = hardfault_args[0];
    volatile uint32_t stacked_r1  = hardfault_args[1];
    volatile uint32_t stacked_r2  = hardfault_args[2];
    volatile uint32_t stacked_r3  = hardfault_args[3];
    volatile uint32_t stacked_r12 = hardfault_args[4];
    volatile uint32_t stacked_lr  = hardfault_args[5];
    volatile uint32_t stacked_pc  = hardfault_args[6];
    volatile uint32_t stacked_psr = hardfault_args[7];

    // Tu peux utiliser printf via UART ou debugger pour afficher les valeurs
    printf("HardFault!\n");
    printf("R0  = 0x%08lX\n", stacked_r0);
    printf("R1  = 0x%08lX\n", stacked_r1);
    printf("R2  = 0x%08lX\n", stacked_r2);
    printf("R3  = 0x%08lX\n", stacked_r3);
    printf("R12 = 0x%08lX\n", stacked_r12);
    printf("LR  = 0x%08lX\n", stacked_lr);
    printf("PC  = 0x%08lX\n", stacked_pc);
    printf("PSR = 0x%08lX\n", stacked_psr);

    // Boucle infinie pour debugger
    while(1);
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/* STM32U5xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32u5xx.s).                    */
/******************************************************************************/


/**
  * @brief This function handles EXTI Line13 interrupt.
  */
void EXTI13_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(CALLBACK_MASTER_TO_SLAVE_Pin);
}

/**
  * @brief This function handles EXTI Line14 interrupt.
  */
void EXTI14_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(MXCHIP_NOTIFY_Pin);
}

/**
  * @brief This function handles EXTI Line15 interrupt.
  */
void EXTI15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(MXCHIP_FLOW_Pin);
}

/**
  * @brief This function handles GPDMA1 Channel 4 global interrupt.
  */
void GPDMA1_Channel4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel4);
}

/**
  * @brief This function handles GPDMA1 Channel 5 global interrupt.
  */
void GPDMA1_Channel5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel5);
}

/**
  * @brief This function handles TIM6 global interrupt.
  */
void TIM6_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
      HAL_IncTick();
  }
  
  if (htim->Instance == TIM2)
  {
      tx_event_flags_set(&tx_led_event, 0x1, TX_OR);
  }
}


/**
  * @brief This function handles SPI2 global interrupt.
  */
void SPI2_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi2);
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles I2C3 Event interrupt.
  */
void I2C3_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&hi2c3);
}

/**
  * @brief This function handles I2C3 Error interrupt.
  */
void I2C3_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&hi2c3);
}

/**
  * @brief This function handles FDCAN1 interrupt 0.
  */
void FDCAN1_IT0_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan1);
}