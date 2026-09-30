add_library(platform_wifi STATIC
    # Composant BSP standard ST (déjà vendorisé dans bsp/stm32cubeu5)
    ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi/mx_wifi.c
    ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi/core/mx_address.c  
    ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi/core/mx_wifi_hci.c
    ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi/core/mx_wifi_ipc.c
    ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi/core/mx_wifi_slip.c

    # Glue SPI board-spécifique
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io_pattern/mx_wifi_spi.c

    # Driver NetX + glue OS ThreadX 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nx_driver_emw3080.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mx_wifi_azure_rtos.c
)

target_include_directories(platform_wifi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/Core/Inc                                    # main.h réel
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/io_pattern 
    ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi
)

target_link_libraries(platform_wifi PUBLIC
    stm32u5xx_hal
    azrtos::threadx
    azrtos::netxduo
)

target_compile_definitions(platform_wifi PRIVATE
    WIFI_SSID="${WIFI_SSID}"
    WIFI_PASSWORD="${WIFI_PASSWORD}"
)
