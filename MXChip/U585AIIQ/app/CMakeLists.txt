
set(SOURCES
    startup/system_stm32u5xx.c
    startup/startup_stm32u585aiixq.s
    startup/tx_initialize_low_level.S
    startup/stm32u5xx_hal_msp.c

    # board_init.c
    # console.c
    # screen.c
    # sntp_client.c
    
    syscalls.c
    sysmem.c
    # wwd_networking.c

    stm32u5xx_hal_timebase_tim.c
    app_threadx.c
    stm32u5xx_it.c
    # tim_callback.c
    main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_arm_post_build_with_usage(${PROJECT_NAME} 2048 786)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
        ${Drivers}/BSP/B-U585I-IOT02A
)

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo
    stm32u5xx_hal
    # mxchip_bsp
    # jsmn
    # wiced_sdk
    # nanoprintf
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/app/startup

)
list(APPEND warnings
  "-Wall"
  "-Wcast-align"
  "-Wcast-qual"
  # "-Wconversion"
  # "-Wctor-dtor-privacy"       # valid for C++/ObjC++ but not for C
  "-Wdangling-else"
  "-Wdouble-promotion"
  "-Wenum-conversion"
  "-Werror"
  "-Wextra"
  # "-Wextra-semi"              # valid for C++/ObjC++ but not for C
  "-Wfatal-errors"
  "-Wimplicit-fallthrough"
  # "-Winvalid-constexpr"       # valid for C++/ObjC++ but not for C
  # "-Wmismatched-tags"         # valid for C++/ObjC++ but not for C
  # "-Wold-style-cast"          # valid for C++/ObjC++ but not for C
  "-Wredundant-decls"
  "-Wshadow"
  # "-Wsign-promo"             # valid for C++/ObjC++ but not for C
  "-Wswitch-enum"
  "-Wundef"
  "-Wuninitialized"
  "-Wunused-const-variable"
  "-Wunused-macros"
  "-Wunused-parameter"
  "-Wunused-variable"
  "-pedantic"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE ${warnings})

target_link_options(${PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/app/startup/STM32U585AIIXQ_FLASH.ld
)

target_link_options(${PROJECT_NAME} PRIVATE -static)

target_link_options(${PROJECT_NAME} PRIVATE -Wl,--print-memory-usage)
