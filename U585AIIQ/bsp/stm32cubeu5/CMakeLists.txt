set(STM32_FAMILY "U5")
set(STM32Cube_DIR "./")

find_package(CMSIS REQUIRED)

set(Drivers ${CMAKE_SOURCE_DIR}/U585AIIQ/bsp/stm32cubeu5/Drivers)
add_subdirectory(Drivers/STM32U5xx_HAL_Driver)
