/**
  ******************************************************************************
  * @file    app_threadx.c
  * @brief   ThreadX applicative file.
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
/* ----------------------- IMPORTANT ORDER ---------------------- */
#include "app_threadx.h"
#include "tx_api.h"
#include "main.h"
#include "app_azure_rtos_config.h"
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
#include "logger.h"

/* Private variables ---------------------------------------------------------*/
static UCHAR tx_app_byte_pool_buffer[TX_APP_MEM_POOL_SIZE] __attribute__((aligned(8)));
static TX_BYTE_POOL tx_app_byte_pool;

static UCHAR nx_app_byte_pool_buffer[NX_APP_MEM_POOL_SIZE] __attribute__((aligned(8)));
static TX_BYTE_POOL nx_app_byte_pool;

/**
  * @brief  Enters the ThreadX kernel.
  * @retval None
  */
VOID MX_ThreadX_Init(VOID)
{
  tx_kernel_enter();
}

/**
  * @brief  ThreadX kernel entry point: creates the application byte pools and
  *         starts the ThreadX-layer and NetXDuo-layer application init.
  * @param  first_unused_memory: unused, required by the ThreadX port.
  * @retval None
  */
VOID tx_application_define(VOID *first_unused_memory)
{
  (void)first_unused_memory;

  UINT status;

  /* ThreadX-layer byte pool: reserved for future threads independent of
     NetXDuo/WiFi (see App_ThreadX_Init()). A failure here is fatal: it means
     the board is out of the small amount of static RAM this pool needs. */
  status = tx_byte_pool_create(&tx_app_byte_pool, "Tx App memory pool",
                                tx_app_byte_pool_buffer, TX_APP_MEM_POOL_SIZE);
  if (status != TX_SUCCESS)
  {
    LOG(FAULT, "Tx App memory pool creation failed: 0x%x\r\n", status);
    Error_Handler();
  }

  status = App_ThreadX_Init(&tx_app_byte_pool);
  if (status != TX_SUCCESS)
  {
    LOG(FAULT, "App_ThreadX_Init failed: 0x%x\r\n", status);
    Error_Handler();
  }

  /* NetXDuo-layer byte pool: backs the main thread, which hosts the
     board's whole business logic (was main()'s while(1) loop). */
  status = tx_byte_pool_create(&nx_app_byte_pool, "Nx App memory pool",
                                nx_app_byte_pool_buffer, NX_APP_MEM_POOL_SIZE);
  if (status != TX_SUCCESS)
  {
    LOG(FAULT, "Nx App memory pool creation failed: 0x%x\r\n", status);
    Error_Handler();
  }

  status = MX_NetXDuo_Init(&nx_app_byte_pool);
  if (status != NX_SUCCESS)
  {
    LOG(FAULT, "MX_NetXDuo_Init failed: 0x%x\r\n", status);
    Error_Handler();
  }
}

/**
  * @brief  ThreadX-layer application init.
  *         This reserves the pool and returns without creating anything.
  * @param  memory_ptr: memory pointer (tx_app_byte_pool)
  * @retval TX_SUCCESS
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  (void)memory_ptr;

  return TX_SUCCESS;
}
