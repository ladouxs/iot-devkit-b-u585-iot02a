set(TARGET_HAL stm32u5xx_hal)

find_package(STM32HAL REQUIRED COMPONENTS dma fdcan flash gpio i2c icache rng rtc spi tim timebase_tim uart usart)
# message("STM32HAL_SOURCES:  ${STM32HAL_SOURCES}")
# message("BSP_SOURCES:  ${BSP_SOURCES}")
# message("BSP_INCLUDE_DIR:  ${BSP_INCLUDE_DIRS}")

add_library(${TARGET_HAL} STATIC 
    ${STM32HAL_SOURCES}
    Src/stm32u5xx_hal_exti.c
    )

# message("config path is : ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/config")

target_include_directories(${TARGET_HAL}
    PUBLIC
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/config
        Inc
)

target_compile_definitions(${TARGET_HAL}
    PUBLIC
        STM32U585xx
        USE_HAL_DRIVER
        STM32U5
)

# CPU / FPU flags (IMPORTANT)
target_compile_options(${TARGET_HAL} 
PRIVATE
    -mcpu=cortex-m33
    -mthumb
)
        
# message("BSP_INCLUDE_DIRS:  ${BSP_INCLUDE_DIRS}")
# message("STM32HAL_INCLUDE_DIR:  ${STM32HAL_INCLUDE_DIR}")
