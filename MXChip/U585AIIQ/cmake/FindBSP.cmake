
# TODO: This could be the subject of a more generic macro (without hard-coded paths).
file(GLOB_RECURSE BSP_SOURCES
    ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/lib/stm32cubeu5/Drivers/BSP/B-U585I-IOT02A/*.c
    ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/lib/stm32cubeu5/Drivers/BSP/Components/*/*.c
)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${curdir}/${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()


SUBDIRLIST(SUBDIRS ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/lib/stm32cubeu5/Drivers/BSP/Components)

# message("SUBDIRS: ${SUBDIRS}")

find_path(
    NAMES b_u585i_iot02a.h
    PATHS ${BSP_BOARD_PATH}}
    HINTS ${CMAKE_SOURCE_DIR}/MXChip/U585AIIQ/lib/stm32cubeu5/Drivers/BSP/B-U585I-IOT02A/
    CMAKE_FIND_ROOT_PATH_BOTH
)

# message("BSP_BOARD_PATH: ${BSP_BOARD_PATH}")

LIST(APPEND SUBDIRS ${BSP_BOARD_PATH})
set(BSP_INCLUDE_DIRS  ${SUBDIRS})

# message("BSP_INCLUDE_DIRS: ${BSP_INCLUDE_DIRS}")

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(BSP DEFAULT_MSG BSP_INCLUDE_DIRS BSP_SOURCES)
