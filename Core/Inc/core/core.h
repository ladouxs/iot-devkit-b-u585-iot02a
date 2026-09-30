#ifndef CORE
#define CORE

#include "stm32u5xx_hal.h"
#include <math.h>
#include <stdbool.h>

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

typedef enum
{
    STATUS_OK,
    STATUS_WARNING,
    STATUS_ERROR
} StatusLevel;

extern UART_HandleTypeDef huart2;

#define DEBUG_MODE 0

#endif /* CORE */
