/**
 ******************************************************************************
 * @file    app_netxduo.c
 * @brief   NetXDuo applicative file.
 *          Hosts the main thread: what used to be main()'s
 *          "USER CODE BEGIN 2" init block and its while(1) loop. Also hosts
 *          the network / WiFi bring-up: packet pool, NetX IP instance (on
 *          the MXCHIP EMW3080 driver), ARP/ICMP/UDP/TCP, DHCP, the NetX BSD
 *          compatibility layer, and a dedicated WiFi bring-up thread.
            init block and its while(1) loop. Never depends on the network.
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

/* --- Network / WiFi (optional) ------------------------------------------- */
/* Everything below is created by WifiThread, not at init time: a failure at
   any step only abandons the WiFi bring-up. */
static TX_BYTE_POOL *NetBytePool = NULL; /* nx_app_byte_pool, kept for WifiThread */

static NX_PACKET_POOL AppPacketPool;

NX_IP IpInstance; /* valid only once App_WiFi_IsReady() has returned NX_TRUE */
static CHAR IpInstanceName[] = "NetX IP Instance 0";
static volatile UINT IpCreated = NX_FALSE;

static NX_DHCP DhcpClient;
static TX_SEMAPHORE DhcpSemaphore;

static TX_THREAD WifiThread;
static CHAR WifiThreadName[] = "WiFi Thread";

/* Private function prototypes -----------------------------------------------*/
static VOID mainThreadEntry(ULONG thread_input);
static VOID WifiThreadEntry(ULONG thread_input);
static UINT WifiBringUp(VOID);
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
 * @retval NX_SUCCESS unless the thread could not be set up.
 */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;
    VOID *stack_ptr = NULL;
    VOID *wifi_stack_ptr = NULL;
    UINT ret;

    NetBytePool = byte_pool;

    nx_system_initialize();

    /* --- main thread stack: reserved first, thread created last --------------------- */
    ret = tx_byte_allocate(byte_pool, &stack_ptr, MAIN_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "main thread stack allocation failed: 0x%x\r\n", ret);
        return ret;
    }

    /* --- WiFi thread: optional, created BEFORE the main thread ---------------- */
    ret = tx_byte_allocate(byte_pool, &wifi_stack_ptr, WIFI_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
    {
        LOG(WARN, "WiFi thread stack allocation failed: 0x%x -- running without WiFi\r\n", ret);
    }
    else
    {
        ret = tx_thread_create(&WifiThread, WifiThreadName, WifiThreadEntry, 0, wifi_stack_ptr, WIFI_THREAD_STACK_SIZE,
                               WIFI_THREAD_PRIORITY, WIFI_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
        if (ret != TX_SUCCESS)
        {
            LOG(WARN, "WiFi thread creation failed: 0x%x -- running without WiFi\r\n", ret);
            (void)tx_byte_release(wifi_stack_ptr);
        }
    }

    /* --- main thread (business logic, unchanged) ------------------------ */
    ret = tx_thread_create(&mainThread, mainThreadName, mainThreadEntry, 0, stack_ptr,
                           MAIN_THREAD_STACK_SIZE, MAIN_THREAD_PRIORITY, MAIN_THREAD_PRIORITY,
                           TX_NO_TIME_SLICE, TX_AUTO_START);
    if (ret != TX_SUCCESS)
    {
        LOG(FAULT, "main thread creation failed: 0x%x\r\n", ret);
        return ret;
    }

    return NX_SUCCESS;
}

/**
 * @brief  Tells whether the network is usable right now: the IP instance exists
 *         and holds a non-zero address (also covers a DHCP lease obtained after
 *         the bring-up timeout, and a lease lost later).
 *         Any code that talks over the network must check this first.
 * @retval NX_TRUE if the network is usable, NX_FALSE otherwise.
 */
UINT App_WiFi_IsReady(VOID)
{
    ULONG ip_address = 0;
    ULONG net_mask = 0;

    if (IpCreated != NX_TRUE)
    {
        return NX_FALSE;
    }

    if (nx_ip_address_get(&IpInstance, &ip_address, &net_mask) != NX_SUCCESS)
    {
        return NX_FALSE;
    }

    return (ip_address != 0) ? NX_TRUE : NX_FALSE;
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

/* Runs one bring-up step; on failure logs it and leaves WifiBringUp() with the
   error code. Needs a local "UINT ret". TX_SUCCESS == NX_SUCCESS == 0. */
#define WIFI_TRY(call, what)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        ret = (call);                                                                                                  \
        if (ret != NX_SUCCESS)                                                                                         \
        {                                                                                                              \
            LOG(WARN, "WiFi: " what " failed: 0x%x\r\n", ret);                                                         \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)

/**
 * @brief  Full WiFi / network bring-up, in thread context (the driver blocks on
 *         semaphores and sleeps while resetting and configuring the module,
 *         which is not allowed during tx_application_define()).
 *
 *         Stops at the first failure and returns its code. Nothing is rolled
 *         back and there is no retry: the driver's one-shot resources (module
 *         byte pool, SPI semaphores/thread) cannot be created twice, so WiFi
 *         stays off until the next reset.
 * @retval NX_SUCCESS if an IP address was obtained, an error code otherwise.
 */
static UINT WifiBringUp(VOID)
{
    VOID *mem = NULL;
    ULONG link_status = 0;
    UINT ret;

    /* --- Packet pool -------------------------------------------------------- */
    WIFI_TRY(tx_byte_allocate(NetBytePool, &mem, NX_PACKET_POOL_SIZE, TX_NO_WAIT), "packet pool allocation");
    WIFI_TRY(nx_packet_pool_create(&AppPacketPool, "Main Packet Pool", PAYLOAD_SIZE, mem, NX_PACKET_POOL_SIZE),
             "nx_packet_pool_create");

    /* --- NetX IP instance, backed by the MXCHIP EMW3080 driver ---------------
       nx_ip_create() runs the driver's initialize step synchronously (module
       hard reset, MX_WIFI_Init): this is the long, failure-prone part. The
       association to WIFI_SSID/WIFI_PASSWORD (driver "enable" step) is run
       afterwards by the IP thread: see the link wait below. */
    WIFI_TRY(tx_byte_allocate(NetBytePool, &mem, NETX_IP_THREAD_STACK_SIZE, TX_NO_WAIT), "IP thread stack allocation");
    WIFI_TRY(nx_ip_create(&IpInstance, IpInstanceName, 0, 0, &AppPacketPool, nx_driver_emw3080_entry, mem,
                          NETX_IP_THREAD_STACK_SIZE, NETX_IP_THREAD_PRIORITY),
             "nx_ip_create (module init)");
    IpCreated = NX_TRUE;

    /* --- ARP / ICMP / UDP / TCP -------------------------------------------- */
    WIFI_TRY(tx_byte_allocate(NetBytePool, &mem, ARP_MEMORY_SIZE, TX_NO_WAIT), "ARP cache allocation");
    WIFI_TRY(nx_arp_enable(&IpInstance, mem, ARP_MEMORY_SIZE), "nx_arp_enable");
    WIFI_TRY(nx_icmp_enable(&IpInstance), "nx_icmp_enable");
    WIFI_TRY(nx_udp_enable(&IpInstance), "nx_udp_enable");
    WIFI_TRY(nx_tcp_enable(&IpInstance), "nx_tcp_enable");

    /* --- DHCP client -------------------------------------------------------- */
    WIFI_TRY(nx_dhcp_create(&DhcpClient, &IpInstance, "DHCP Client"), "nx_dhcp_create");
    WIFI_TRY(tx_semaphore_create(&DhcpSemaphore, "DHCP Semaphore", 0), "DHCP semaphore creation");
    WIFI_TRY(nx_ip_address_change_notify(&IpInstance, ip_address_change_notify_callback, NULL),
             "nx_ip_address_change_notify");

    /* --- NetX BSD compatibility layer ---------------------------------------
       Runs on its own internal thread; gives the rest of the firmware a
       standard socket()/connect()/send()/recv() API on top of NetX. */
    WIFI_TRY(tx_byte_allocate(NetBytePool, &mem, BSD_COMPAT_LAYER_THREAD_STACK_SIZE, TX_NO_WAIT),
             "BSD layer stack allocation");
    WIFI_TRY(nx_bsd_initialize(&IpInstance, &AppPacketPool, (CHAR *)mem, BSD_COMPAT_LAYER_THREAD_STACK_SIZE,
                               BSD_COMPAT_LAYER_THREAD_PRIORITY),
             "nx_bsd_initialize");

    /* --- Wait (bounded) for the link: SSID association done by the IP thread -- */
    ret = nx_ip_status_check(&IpInstance, NX_IP_LINK_ENABLED, &link_status,
                             WIFI_LINK_TIMEOUT_SECONDS * TX_TIMER_TICKS_PER_SECOND);
    if (ret != NX_SUCCESS)
    {
        LOG(WARN, "WiFi: link not enabled after %d s (SSID '%s' not found / wrong password?)\r\n",
            WIFI_LINK_TIMEOUT_SECONDS,
            WIFI_SSID
        );
        return ret;
    }

    /* --- DHCP lease (bounded) ----------------------------------------------- */
    WIFI_TRY(nx_dhcp_start(&DhcpClient), "nx_dhcp_start");

    ret = tx_semaphore_get(&DhcpSemaphore, WIFI_LINK_TIMEOUT_SECONDS * TX_TIMER_TICKS_PER_SECOND);
    if (ret != TX_SUCCESS)
    {
        /* The DHCP client keeps running in the background: a lease obtained
           later is still picked up by App_WiFi_IsReady(). */
        LOG(WARN, "WiFi: no IP address after %d s\r\n", WIFI_LINK_TIMEOUT_SECONDS);
        return ret;
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

    return NX_SUCCESS;
}

#undef WIFI_TRY

/**
 * @brief  WiFi thread entry: runs the whole bring-up once, then ends.
 *         Runs at a higher priority than mainThread (see WIFI_THREAD_PRIORITY)
 *         so it starts first and cannot be starved by the main loop. A failure
 *         is only logged: mainThread never waits for, or depends on, it.
 * @param  thread_input: unused
 * @retval None
 */
static VOID WifiThreadEntry(ULONG thread_input)
{
    UINT ret;

    (void)thread_input;

    ret = WifiBringUp();
    if (ret != NX_SUCCESS)
    {
        LOG(WARN, "WiFi unavailable (0x%x): network communication disabled, business logic unaffected\r\n", ret);
    }
}

/**
 * @brief  main thread entry.
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
