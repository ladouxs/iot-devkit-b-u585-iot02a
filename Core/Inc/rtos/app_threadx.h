/**
  ******************************************************************************
  * @file    app_threadx.h
  * @brief   ThreadX applicative header file.
  ******************************************************************************
  * @attention
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_THREADX_H
#define __APP_THREADX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Enters the ThreadX kernel. Never returns.
  */
VOID MX_ThreadX_Init(VOID);

/**
  * @brief  ThreadX-layer application init, called from tx_application_define().
  *         Reserved for threads that must run independently of the
  *         NetXDuo/WiFi stack.
  * @param  memory_ptr: memory pointer (tx_app_byte_pool)
  * @retval TX_SUCCESS on success, a ThreadX error code otherwise.
  */
UINT App_ThreadX_Init(VOID *memory_ptr);

#ifdef __cplusplus
}
#endif

#endif /* __APP_THREADX_H */
