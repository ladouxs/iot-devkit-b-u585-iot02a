add_library(platform_hal STATIC
    src/stm32u5xx_hal_msp.c
    # src/stm32u5xx_hal_timebase_tim.c
    # src/stm32u5xx_it.c <- dans l'app pour être pris en compte par le linker
    src/syscalls.c
    src/sysmem.c
)

target_include_directories(platform_hal
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_SOURCE_DIR}/U585AIIQ/app/inc   # for main.h
        ${CMAKE_SOURCE_DIR}/U585AIIQ/platform/error/inc
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/STM32U5xx_HAL_Driver/Inc
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/CMSIS/Device/ST/STM32U5xx/Include
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/CMSIS/Include
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/Components/mx_wifi/io_pattern
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers/BSP/B-U585I-IOT02A/
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/config/
        ${CMAKE_SOURCE_DIR}/U585AIIQ/platform/wifi/inc
        ${CMAKE_SOURCE_DIR}/shared/lib/netxduo/common/drivers/wifi/mxchip
)

target_link_libraries(platform_hal
    PUBLIC
        stm32u5xx_hal
        azrtos::threadx
        azrtos::netxduo
)

target_compile_features(platform_hal PRIVATE c_std_11)
