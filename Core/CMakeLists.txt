

# ---------------------------------------------------------------------------
# CubeMX-generated application / platform sources
# ---------------------------------------------------------------------------

target_sources(${PROJECT_NAME}
    PRIVATE
        Src/main.c
        Src/stm32u5xx_it.c
        Src/stm32u5xx_hal_msp.c
        Src/stm32u5xx_hal_timebase_tim.c
        Src/system_stm32u5xx.c
        Src/sysmem.c
        Src/syscalls.c
)

# ---------------------------------------------------------------------------
# ThreadX / NetXDuo application sources
# ---------------------------------------------------------------------------

target_sources(${PROJECT_NAME}
    PRIVATE
        Src/rtos/app_threadx.c
        Src/rtos/app_netxduo.c
        # ThreadX Cortex-M33/GNU port glue: not part of azrtos::threadx's own
        # build (ThreadX ships tx_initialize_low_level.S only as integrator-
        # supplied BSP code under ports/.../example_build/).
        Src/rtos/tx_initialize_low_level.S
        Src/rtos/tx_thread_secure_stack_initialize.S
)

# ---------------------------------------------------------------------------
# Application sources
# ---------------------------------------------------------------------------

target_sources(${PROJECT_NAME}
    PRIVATE
        Src/communication/uart.c
        Src/core/logger.c

        Src/sensors/ADS1115.c
        Src/sensors/driver_ina219.c
        Src/sensors/driver_ina219_basic.c
        Src/sensors/driver_ina219_interface_template.c

)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME}
    PRIVATE
        Inc
        Inc/rtos
        Inc/communication
        Inc/core

        Inc/sensors
)
