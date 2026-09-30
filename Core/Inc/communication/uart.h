#ifndef UART
#define UART

#include "main.h"
// Forward declaration to avoid circular dependency with core.h
#include "stm32u5xx_hal_def.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#define UART_BUFFER_SIZE 256

extern UART_HandleTypeDef huart1;

void sendUSART1Message(const char *format, ...);
#endif /* UART */
