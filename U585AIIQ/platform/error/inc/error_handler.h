#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void Fatal_Error(uint32_t error_code);

void Error_Handler(void);

void Success_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_HANDLER_H */
