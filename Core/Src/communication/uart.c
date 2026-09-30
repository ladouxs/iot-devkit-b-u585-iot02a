#include "uart.h"
#include "logger.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

// Transmet un message UART basé sur le niveau et le type
void sendStatusMessage(char buffer[UART_BUFFER_SIZE], const char *type, int level)
{
    const char *levelStr = (level == 1) ? "WARNING" : "ERROR";
    int len = snprintf(buffer, UART_BUFFER_SIZE, "STATUS_%s_%s\r\n", levelStr, type);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, len, HAL_MAX_DELAY);
}

void sendUSART1Message(const char *format, ...)
{
    char message[256];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message, 256, format, ap);
    uint16_t len = strlen(message);

    HAL_UART_Transmit(&huart1, (uint8_t *)message, len, 1000);

    va_end(ap);
}