# Define ThreadX user configuration
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")

# Define NetXDuo user configuration
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/netxduo/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NXD_ENABLE_FILE_SERVERS OFF CACHE BOOL "Disable fileX dependency by netxduo")

# Core libraries
add_subdirectory(${SHARED_LIB_DIR}/threadx threadx)
add_subdirectory(${SHARED_LIB_DIR}/netxduo netxduo)
add_subdirectory(stm32cubeu5)

# add_subdirectory(mxchip_bsp)
# add_subdirectory(wiced_sdk)
