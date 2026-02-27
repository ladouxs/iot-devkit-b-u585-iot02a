set(STM32_FAMILY "U5")
set(STM32Cube_DIR "./")

find_package(CMSIS REQUIRED)
# message("CMSIS_INCLUDE_DIRS:  ${CMSIS_INCLUDE_DIRS}")

set(Drivers ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/lib/stm32cubeu5/Drivers)
add_subdirectory(Drivers/STM32U5xx_HAL_Driver)

# add_subdirectory(BSP)