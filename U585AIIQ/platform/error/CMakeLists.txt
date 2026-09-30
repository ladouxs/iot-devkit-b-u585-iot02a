add_library(platform_error STATIC
    src/error_handler.c
)

target_include_directories(platform_error PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/U585AIIQ/app/inc

    ${CMAKE_SOURCE_DIR}/shared/lib/threadx/ports/cortex_m33/gnu/inc
    ${CMAKE_SOURCE_DIR}/shared/lib/threadx/common/inc
)

target_link_libraries(platform_error PUBLIC stm32u5xx_hal)

target_compile_options(platform_error PRIVATE
    -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard
)