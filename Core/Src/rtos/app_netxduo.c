/**
 ******************************************************************************
 * @file    app_netxduo.c
 * @brief   NetXDuo applicative file.
 *          Hosts the main thread: what used to be main()'s
 *          "USER CODE BEGIN 2" init block and its while(1) loop. Also hosts
 *          the network / WiFi bring-up: packet pool, NetX IP instance (on
 *          the MXCHIP EMW3080 driver), ARP/ICMP/UDP/TCP, DHCP, the NetX BSD
 *          compatibility layer, and a dedicated WiFi bring-up thread.
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
#include "app_netxduo.h"
#include "app_azure_rtos_config.h"
#include "app_threadx.h"
#include "main.h"

/* Network / WiFi includes ----------------------------------------------------*/
#include "io_pattern/mx_wifi_io.h" /* wifi_obj_get() */
#include "mx_wifi.h"               /* wifi_obj_get() */
#include "nx_driver_emw3080.h"     /* nx_driver_emw3080_entry() */
#include "nxd_bsd.h"               /* NetX BSD compatibility layer */
#include "nxd_dhcp_client.h"

/* Private includes ----------------------------------------------------------*/
#include "ADS1115.h"
#include "driver_ina219.h"
#include "driver_ina219_basic.h"
#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>

/* Private variables ---------------------------------------------------------*/
static TX_THREAD mainThread;
static CHAR mainThreadName[] = "Main Thread";

/* Defined here (was a plain global in main.c): FDCAN1 was already brought up
   by MX_FDCAN1_Init() in main(), before MX_ThreadX_Init() is called. */
extern FDCAN_HandleTypeDef hfdcan1;

extern RNG_HandleTypeDef hrng;

/* --- Network / WiFi bring-up ------------------------------------------- */
static NX_PACKET_POOL AppPacketPool;

NX_IP IpInstance;
static CHAR IpInstanceName[] = "NetX IP Instance 0";

static NX_DHCP DhcpClient;
static TX_SEMAPHORE DhcpSemaphore;

static TX_THREAD WifiThread;
static CHAR WifiThreadName[] = "WiFi Thread";

/* Private function prototypes -----------------------------------------------*/
static VOID mainThreadEntry(ULONG thread_input);
static VOID WifiThreadEntry(ULONG thread_input);
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);

int hardware_rand(void)
{
    uint32_t random_value = 0;
    (void)HAL_RNG_GenerateRandomNumber(&hrng, &random_value);
    return (int)random_value;
}

/**
 * @brief  NetXDuo-layer application init.
 * @param  memory_ptr: memory pointer (nx_app_byte_pool)
 * @retval NX_SUCCESS on success, an error code otherwise.
 */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;
    VOID *stack_ptr = NULL;
    UINT ret;

    nx_system_initialize();

    /* --- Packet pool ------------------------------------------------------ */
    {
        VOID *pool_start;

        ret = tx_byte_allocate(byte_pool, &pool_start, NX_PACKET_POOL_SIZE, TX_NO_WAIT);
        if (ret != TX_SUCCESS)
        {
            LOG(FAULT, "Packet pool allocation failed: 0x%x\r\n", ret);
            return ret;
        }

        ret = nx_packet_pool_create(&AppPacketPool, "Main Packet Pool", PAYLOAD_SIZE, pool_start, NX_PACKET_POOL_SIZE);
        if (ret != NX_SUCCESS)
        {
            LOG(FAULT, "nx_packet_pool_create failed: 0x%x\r\n", ret);
            return ret;
        }
    }

    /* --- NetX IP instance, backed by the MXCHIP EMW3080 driver ------------
       nx_ip_create() starts the driver's own entry point (nx_driver_emw3080_entry),
       which brings up the WiFi module and associates it to WIFI_SSID/WIFI_PASSWORD
       (see mx_wifi_conf.h) as part of its initialize/enable sequence. */
    {
        ret = tx_byte_allocate(byte_pool, &stack_ptr, NETX_IP_THREAD_STACK_SIZE, TX_NO_WAIT);
        if (ret != TX_SUCCESS)
        {
            LOG(FAULT, "IP thread stack allocation failed: 0x%x\r\n", ret);
            return ret;
        }

        ret = nx_ip_create(&IpInstance, IpInstanceName, 0, 0, &AppPacketPool, nx_driver_emw3080_entry, stack_ptr,
                           NETX_IP_THREAD_STACK_SIZE, NETX_IP_THREAD_PRIORITY);
        if (ret != NX_SUCCESS)
        {
            LOG(FAULT, "nx_ip_create failed: 0x%x\r\n", ret);
            return ret;
        }
    }

    /* --- ARP / ICMP / UDP / TCP -------------------------------------------- */
    {
        VOID *arp_cache_memory;

        ret = tx_byte_allocate(byte_pool, &arp_cache_memory, ARP_MEMORY_SIZE, TX_NO_WAIT);
        if (ret != TX_SUCCESS)
        {
            LOG(FAULT, "ARP cache allocation failed: 0x%x\r\n", ret);
            return ret;
        }

        ret = nx_arp_enable(&IpInstance, arp_cache_memory, ARP_MEMORY_SIZE);
        if (ret != NX_SUCCESS)
        {
            LOG(FAULT, "nx_arp_enable failed: 0x%x\r\n", ret);
            return ret;
        }
    }

    ret = nx_icmp_enable(&IpInstance);
    if (ret != NX_SUCCESS)
    {
        LOG(FAULT, "nx_icmp_enable failed: 0x%x\r\n", ret);
        return ret;
    }

    ret = nx_udp_enable(&IpInstance);
    if (ret != NX_SUCCESS)
    {
        LOG(FAULT, "nx_udp_enable failed: 0x%x\r\n", ret);
        return ret;
    }

    ret = nx_tcp_enable(&IpInstance);
    if (ret != NX_SUCCESS)
    {
        LOG(FAULT, "nx_tcp_enable failed: 0x%x\r\n", ret);
        return ret;
    }

    /* --- DHCP client -------------------------------------------------------- */
    ret = nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client");
    if (ret != NX_SUCCESS)
    {
        LOG(FAULT, "nx_dhcp_create failed: 0x%x\r\n", ret);
        return ret;
    }

    ret = tx_semaphore_create(&DhcpSemaphore, "DHCP Semaphore", 0);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "DHCP semaphore creation failed: 0x%x\r\n", ret);
        return ret;
    }

    /* --- NetX BSD compatibility layer ---------------------------------------
       Runs on its own internal thread; gives the rest of the firmware a
       standard socket()/connect()/send()/recv() API on top of NetX. */
    {
        VOID *bsd_stack_ptr;

        ret = tx_byte_allocate(byte_pool, &bsd_stack_ptr, BSD_COMPAT_LAYER_THREAD_STACK_SIZE, TX_NO_WAIT);
        if (ret != TX_SUCCESS)
        {
            LOG(FAULT, "BSD layer stack allocation failed: 0x%x\r\n", ret);
            return ret;
        }

        ret = nx_bsd_initialize(&IpInstance, &AppPacketPool, (CHAR *)bsd_stack_ptr, BSD_COMPAT_LAYER_THREAD_STACK_SIZE,
                                BSD_COMPAT_LAYER_THREAD_PRIORITY);
        if (ret != NX_SUCCESS)
        {
            LOG(FAULT, "nx_bsd_initialize failed: 0x%x\r\n", ret);
            return ret;
        }
    }

    /* --- main thread (business logic, unchanged) ------------------------ */
    ret = tx_byte_allocate(byte_pool, &stack_ptr, MAIN_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "main thread stack allocation failed: 0x%x\r\n", ret);
        return ret;
    }

    ret = tx_thread_create(&mainThread, mainThreadName, mainThreadEntry, 0, stack_ptr,
                           MAIN_THREAD_STACK_SIZE, MAIN_THREAD_PRIORITY, MAIN_THREAD_PRIORITY,
                           TX_NO_TIME_SLICE, TX_AUTO_START);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "main thread creation failed: 0x%x\r\n", ret);
        return ret;
    }

    /* --- WiFi bring-up thread: separate from mainThread ------------------- */
    ret = tx_byte_allocate(byte_pool, &stack_ptr, WIFI_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "WiFi thread stack allocation failed: 0x%x\r\n", ret);
        return ret;
    }

    ret = tx_thread_create(&WifiThread, WifiThreadName, WifiThreadEntry, 0, stack_ptr, WIFI_THREAD_STACK_SIZE,
                           WIFI_THREAD_PRIORITY, WIFI_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "WiFi thread creation failed: 0x%x\r\n", ret);
        return ret;
    }

    return NX_SUCCESS;
}

/**
 * @brief  IP address change callback: releases the DHCP semaphore as soon
 *         as the IP instance has an address (lease acquired or renewed).
 * @param  ip_instance: NX_IP instance
 * @param  ptr: user data (unused)
 * @retval None
 */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
    (void)ip_instance;
    (void)ptr;

    tx_semaphore_put(&DhcpSemaphore);
}

/**
 * @brief  WiFi bring-up thread entry: starts DHCP and waits (bounded) for a
 *         lease, then logs the assigned address and module info. Runs
 *         independently of mainThread: business logic starts and
 *         keeps running whether or not -- and however long -- the network
 *         takes to come up.
 * @param  thread_input: unused
 * @retval None
 */
static VOID WifiThreadEntry(ULONG thread_input)
{
    UINT ret;

    (void)thread_input;

    ret = nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL);
    if (ret != NX_SUCCESS)
    {
        LOG(FAULT, "nx_ip_address_change_notify failed: 0x%x -- aborting WiFi thread\r\n", ret);
        return;
    }

    ret = nx_dhcp_start(&DhcpClient);
    if (ret != NX_SUCCESS)
    {
        LOG(FAULT, "nx_dhcp_start failed: 0x%x -- aborting WiFi thread\r\n", ret);
        return;
    }

    ret = tx_semaphore_get(&DhcpSemaphore, WIFI_DHCP_TIMEOUT_SECONDS * TX_TIMER_TICKS_PER_SECOND);
    if (ret != TX_SUCCESS)
    {
        LOG(WARN, "WiFi: no IP address after %d s -- giving up, business logic unaffected\r\n", WIFI_DHCP_TIMEOUT_SECONDS);
        return;
    }

    {
        ULONG ip_address = 0;
        ULONG net_mask = 0;
        ULONG gateway_address = 0;

        (void)nx_ip_address_get(&IpInstance, &ip_address, &net_mask);
        (void)nx_ip_gateway_address_get(&IpInstance, &gateway_address);

        LOG(INFO, "WiFi connected: IP %lu.%lu.%lu.%lu  mask %lu.%lu.%lu.%lu  gw %lu.%lu.%lu.%lu\r\n",
            (ip_address >> 24) & 0xFF, (ip_address >> 16) & 0xFF, (ip_address >> 8) & 0xFF, ip_address & 0xFF,
            (net_mask >> 24) & 0xFF, (net_mask >> 16) & 0xFF, (net_mask >> 8) & 0xFF, net_mask & 0xFF,
            (gateway_address >> 24) & 0xFF, (gateway_address >> 16) & 0xFF, (gateway_address >> 8) & 0xFF,
            gateway_address & 0xFF);

        LOG(INFO, "WiFi module: %s / %s, FW %s, MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
            wifi_obj_get()->SysInfo.Product_Name, wifi_obj_get()->SysInfo.Product_ID, wifi_obj_get()->SysInfo.FW_Rev,
            wifi_obj_get()->SysInfo.MAC[0], wifi_obj_get()->SysInfo.MAC[1], wifi_obj_get()->SysInfo.MAC[2],
            wifi_obj_get()->SysInfo.MAC[3], wifi_obj_get()->SysInfo.MAC[4], wifi_obj_get()->SysInfo.MAC[5]);
    }
}

/**
 * @brief  Main thread entry.
 *         Was main()'s "USER CODE BEGIN 2" init block, followed by its
 *         while(1) loop. 
 * @param  thread_input: unused
 * @retval None
 */
static VOID mainThreadEntry(ULONG thread_input)
{
    (void)thread_input;

    /* USER CODE BEGIN 2 (relocated from main.c) */
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_13, GPIO_PIN_SET); /* Activate UART2 and I2C pin communication */
    ina219_basic_init(INA219_SENSOR_MAIN, INA219_ADDRESS_0, 0.1);
    ina219_basic_init(INA219_SENSOR_SECONDARY, INA219_ADDRESS_4, 0.1);

    LOG(INFO, "Started successfully )(. Version %d.%d.%d", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
    /* USER CODE END 2 */

    /* Infinite loop (relocated from main.c's "USER CODE BEGIN WHILE") */
    while (1)
    {
    }
}
