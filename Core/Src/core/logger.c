#include "logger.h"

static char *names[LOG_LEVEL_COUNT] = {
    [FAULT] = "[FAULT]",
    [WARN] = "[WARN]",
    [INFO] = "[INFO]",
    [DEBUG] = "[DEBUG]",
};

static char *colors[LOG_LEVEL_COUNT] = {
    [FAULT] = ANSI_BOLD_RED,
    [WARN] = ANSI_BOLD_YELLOW,
    [INFO] = ANSI_BOLD_GREEN,
    [DEBUG] = ANSI_BOLD_BLUE,
};

void LOG(enum LOG_LEVEL level, const char *format, ...)
{
    if (level == DEBUG && DEBUG_MODE == 0)
        return;
    char message[256];
    va_list ap;
    va_start(ap, format);

    vsnprintf(message, 256, format, ap);
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    sendUSART1Message(ANSI_CYAN "[%02u:%02u:%02u]" ANSI_RESET " %s%s" ANSI_RESET " " ANSI_WHITE "%s" ANSI_RESET "\r\n",
                      sTime.Hours, sTime.Minutes, sTime.Seconds, colors[level], names[level], message);
    va_end(ap);
}