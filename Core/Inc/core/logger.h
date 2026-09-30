#ifndef LOGGER
#define LOGGER

#include "core.h"
#include "stm32u5xx_hal.h"
#include "uart.h"

#define LOG_LEVEL_COUNT 4

#define ANSI_RESET "\033[0m"
#define ANSI_CYAN "\033[36m"
#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_YELLOW "\033[1;33m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_BLUE "\033[1;34m"
#define ANSI_WHITE "\033[1;37m"

enum LOG_LEVEL
{
    /**
     * @brief Error wich do not make the system crash but some parts of it might
     * not work.
     */
    FAULT,
    /**
     * @brief Event that are not optimal and should be avoided but has little to
     * no effect for the system.
     */
    WARN,
    /**
     * @brief Informations.
     */
    INFO,
    /**
     * @brief Details about the systems.
     */
    DEBUG
};

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

/**
 * @brief Log a message with the given level over USART1.
 * @note The timestamp start from 00:00:00 when the board is flashed.
 *
 * @param level The priority level of the log.
 * @param format The log message to be displayed.
 *
 */
void LOG(enum LOG_LEVEL level, const char *format, ...);

#endif /* LOGGER */
