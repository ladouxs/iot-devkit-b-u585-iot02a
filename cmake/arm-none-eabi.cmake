# ============================================================
# Toolchain ARM bare-metal STM32U585
# ============================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING TRUE)

# Toolchain
set(TOOLCHAIN_PREFIX arm-none-eabi)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU flags
set(MCU_FLAGS "-mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
# set(MCU_FLAGS "-mcpu=cortex-m33 -mthumb")

set(CMAKE_C_FLAGS_INIT   "${MCU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS} -x assembler-with-cpp")

add_compile_definitions(STM32U585xx)

# Build type by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()
    
# Binary tools
execute_process(
    COMMAND which ${TOOLCHAIN_PREFIX}-gcc
    OUTPUT_VARIABLE ARM_GCC
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

cmake_path(GET ARM_GCC PARENT_PATH ARM_BIN_DIR)

set(CMAKE_OBJCOPY ${ARM_BIN_DIR}/${TOOLCHAIN_PREFIX}-objcopy)
set(CMAKE_SIZE    ${ARM_BIN_DIR}/${TOOLCHAIN_PREFIX}-size)

# ThreadX
set(THREADX_ARCH cortex_m33)
set(THREADX_TOOLCHAIN gnu)

# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)