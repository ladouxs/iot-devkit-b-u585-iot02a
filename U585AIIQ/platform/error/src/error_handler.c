#include "error_handler.h"

#include "stm32u5xx_hal.h"
#include "tx_api.h"
#include "main.h"


void Fatal_Error(uint32_t error_code)
{
    __disable_irq();

    (void)error_code;

    // [TODO] store error_code in RAM backup

    // Send to UART if available
    printf("An error occured\n");

    // [TODO] log to a Flash memory area

#ifdef DEBUG

    /* Debug mode: infinite LED loop */
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        for (volatile uint32_t i = 0; i < 500000; i++);
    }

#else

    /* Release mode: Blink red led then immediate reset */
    for(unsigned long j=0; j<20; j++)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        for (volatile uint32_t i = 0; i < 500000; i++);
    }
    NVIC_SystemReset();

#endif
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    Fatal_Error(0xDEAD0001);
}

void Success_Handler(void)
{
   HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
   while(1)
   {
     HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
     tx_thread_sleep(50);
   }
}
