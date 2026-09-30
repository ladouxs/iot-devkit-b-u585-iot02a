include(cmake/ProjectCompileOptions.cmake)

add_subdirectory(bsp)
add_subdirectory(platform/wifi)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        stm32u5xx_hal
        platform_wifi
)
