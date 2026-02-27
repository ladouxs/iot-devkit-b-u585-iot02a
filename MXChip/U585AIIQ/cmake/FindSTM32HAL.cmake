#  Copyright (c) Microsoft
#  Copyright (c) 2024 Eclipse Foundation
# 
#  This program and the accompanying materials are made available 
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
# 
#  SPDX-License-Identifier: MIT
# 
#  Contributors: 
#     Microsoft         - Initial version
#     Frédéric Desbiens - 2024 version.

set(HAL_COMPONENTS 
adc adc_ex cortex crc cryp cryp_ex dcmi dma dma_ex dsi exti flash flash_ex gpio gtzc hash hash_ex hcd i2c i2c_ex icache iwdg jpeg lptim ltdc ltdc_ex 
mdma mmc mmc_ex opamp opamp_ex ospi pka pcd pcd_ex pwr pwr_ex ramcfg rcc rcc_ex rng rng_ex rtc rtc_ex sai sai_ex sd sd_ex smartcard smartcard_ex 
smbus smbus_ex spi spi_ex tim tim_ex timebase_tim uart uart_ex usart usart_ex usb wwdg
)

set(HAL_REQUIRED_COMPONENTS cortex pwr rcc)

# Components that have _ex sources
set(HAL_EX_COMPONENTS adc crc cryp dac dma flash hash i2c ltdc mmc opamp pcd pwr rcc rng rtc sai sd smartcard smbus spi tim uart usart)

set(HAL_PREFIX stm32u5xx_)

set(HAL_HEADERS
	${HAL_PREFIX}hal.h
	${HAL_PREFIX}hal_def.h
)

set(HAL_SRCS
	${HAL_PREFIX}hal.c
)

if(NOT STM32HAL_FIND_COMPONENTS)
    set(STM32HAL_FIND_COMPONENTS ${HAL_COMPONENTS})
    message(STATUS "No STM32HAL components selected, using all: ${STM32HAL_FIND_COMPONENTS}")
else()
    message(STATUS "STM32HAL components selected, using: ${STM32HAL_FIND_COMPONENTS}")
endif()

foreach(cmp ${HAL_REQUIRED_COMPONENTS})
    list(FIND STM32HAL_FIND_COMPONENTS ${cmp} STM32HAL_FOUND_INDEX)
    if(${STM32HAL_FOUND_INDEX} LESS 0)
        list(APPEND STM32HAL_FIND_COMPONENTS ${cmp})
    endif()
endforeach()

foreach(cmp ${STM32HAL_FIND_COMPONENTS})
    list(FIND HAL_COMPONENTS ${cmp} STM32HAL_FOUND_INDEX)
    if(${STM32HAL_FOUND_INDEX} LESS 0)
        message(FATAL_ERROR "Unknown STM32HAL component: ${cmp}. Available components: ${HAL_COMPONENTS}")
	else()
        list(APPEND HAL_HEADERS ${HAL_PREFIX}hal_${cmp}.h)
        list(APPEND HAL_SRCS ${HAL_PREFIX}hal_${cmp}.c)
        endif()
    list(FIND HAL_EX_COMPONENTS ${cmp} STM32HAL_FOUND_INDEX)
    if(NOT (${STM32HAL_FOUND_INDEX} LESS 0))
        list(APPEND HAL_HEADERS ${HAL_PREFIX}hal_${cmp}_ex.h)
        list(APPEND HAL_SRCS ${HAL_PREFIX}hal_${cmp}_ex.c)
    endif()
endforeach()

list(REMOVE_DUPLICATES HAL_HEADERS)
list(REMOVE_DUPLICATES HAL_SRCS)

# message("STM32HAL_INCLUDE_DIR HINTS ${Drivers}/STM32${STM32_FAMILY}xx_HAL_Driver/Src")


find_path(STM32HAL_INCLUDE_DIR ${HAL_HEADERS}
    HINTS ${Drivers}/STM32${STM32_FAMILY}xx_HAL_Driver/Inc
    CMAKE_FIND_ROOT_PATH_BOTH
)

# message("HAL_SRC HINTS ${Drivers}/STM32${STM32_FAMILY}xx_HAL_Driver/Src")

foreach(HAL_SRC ${HAL_SRCS})
    string(MAKE_C_IDENTIFIER "${HAL_SRC}" HAL_SRC_CLEAN)
    set(HAL_${HAL_SRC_CLEAN}_FILE HAL_SRC_FILE-NOTFOUND)
    find_file(HAL_${HAL_SRC_CLEAN}_FILE ${HAL_SRC}
        HINTS ${Drivers}/STM32${STM32_FAMILY}xx_HAL_Driver/Src
        CMAKE_FIND_ROOT_PATH_BOTH
    )
    list(APPEND STM32HAL_SOURCES ${HAL_${HAL_SRC_CLEAN}_FILE})
endforeach()

# message("STM32HAL_SOURCES: ${STM32HAL_SOURCES}")

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(STM32HAL DEFAULT_MSG STM32HAL_INCLUDE_DIR STM32HAL_SOURCES)
