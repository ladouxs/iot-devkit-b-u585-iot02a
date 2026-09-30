# Define ThreadX user configuration
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")

# Define NetXDuo user configuration
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo")


# Core libraries
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../shared/lib/threadx
    threadx
)

add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../shared/lib/netxduo
    netxduo
)

add_subdirectory(stm32cubeu5)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME}
    PRIVATE
        stm32cubeu5/Drivers/STM32U5xx_HAL_Driver/Inc
        stm32cubeu5/Drivers/STM32U5xx_HAL_Driver/Inc/Legacy
        stm32cubeu5/Drivers/CMSIS/Device/ST/STM32U5xx/Include
        stm32cubeu5/Drivers/CMSIS/Include
        config
)

# ---------------------------------------------------------------------------
# ThreadX / NetXDuo link
# ---------------------------------------------------------------------------

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        azrtos::threadx
        azrtos::netxduo
)
