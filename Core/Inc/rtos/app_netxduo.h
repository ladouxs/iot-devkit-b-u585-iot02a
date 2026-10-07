/**
 ******************************************************************************
 * @file    app_netxduo.h
 * @brief   NetXDuo applicative header file.
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
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"
#include "tx_api.h"

/* Exported constants --------------------------------------------------------*/

/* main thread: hosts what used to be main()'s "USER CODE BEGIN 2" block
 * (sensor / CAN bring-up) and its while(1) loop (EVSE handling, data
 * acquisition, EKF update, CAN + UART reporting). Sized generously: the
 * loop keeps a UART formatting buffer and several float locals on the
 * stack. */
#define MAIN_THREAD_STACK_SIZE 4096
#define MAIN_THREAD_PRIORITY 10

    /* --- Network / WiFi bring-up -----------------------------------------------
     * a packet pool + NetX IP instance backed by the MXCHIP EMW3080 driver (nx_driver_emw3080_entry),
     * ARP/ICMP/UDP/TCP, a DHCP client, the NetX BSD compatibility layer
     * (nx_bsd_initialize), and a dedicated WiFi bring-up thread, independent
     * from mainThread so business logic never waits on the network.
     *
     * The DHCP wait in WifiThreadEntry() is itself time-bounded (WIFI_LINK_TIMEOUT_SECONDS)
     * instead of TX_WAIT_FOREVER, which makes a second thread unnecessary.
     * WiFi module init/association (MX_WIFI_Init()/MX_WIFI_Connect(), driven by
     * WIFI_SSID/WIFI_PASSWORD -- see U585AIIQ/platform/wifi/inc/mx_wifi_conf.h)
     * happens synchronously inside nx_ip_create(), before this thread even
     * starts: see nx_driver_emw3080_entry()/_nx_driver_emw3080_initialize()/
     * _nx_driver_emw3080_enable(). */

#define PAYLOAD_SIZE 1544
#define NX_PACKET_POOL_SIZE ((PAYLOAD_SIZE + sizeof(NX_PACKET)) * 20)

#define ARP_MEMORY_SIZE 1024

    // #define NETX_IP_THREAD_STACK_SIZE 2048
    // #define NETX_IP_THREAD_PRIORITY 2

#define BSD_COMPAT_LAYER_THREAD_STACK_SIZE 2048
#define BSD_COMPAT_LAYER_THREAD_PRIORITY 3

#define WIFI_THREAD_STACK_SIZE 4096
/* Numerically higher than MAIN_THREAD_PRIORITY -- i.e. LOWER priority:
 * Business logic always preempts WiFi bring-up, never the other way around. */
#define WIFI_THREAD_PRIORITY 12

/* How long the WiFi thread waits for the link (SSID association, done by the
 * IP thread) before giving up. */
#define WIFI_LINK_TIMEOUT_SECONDS 15

    /* Exported functions prototypes ---------------------------------------------*/

    /**
     * @brief  NetXDuo-layer application init: brings up the packet pool, the
     *         NetX IP instance (which brings up the WiFi module and associates
     *         it to WIFI_SSID/WIFI_PASSWORD as a side effect of nx_ip_create()),
     *         ARP/ICMP/UDP/TCP, the DHCP client, the NetX BSD compatibility
     *         layer, the main thread, and the WiFi bring-up thread.
     * @param  memory_ptr: memory pointer (nx_app_byte_pool)
     * @retval NX_SUCCESS on success, an error code otherwise.
     */
    UINT MX_NetXDuo_Init(VOID *memory_ptr);

    /* Exported variables ---------------------------------------------------------*/
    extern NX_IP IpInstance;

#ifdef __cplusplus
}
#endif

#endif /* __APP_NETXDUO_H__ */