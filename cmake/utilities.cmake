
function(set_target_linker TARGET LINKER_SCRIPT)
    if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
        target_link_options(${TARGET} PRIVATE --config ${LINKER_SCRIPT})
        target_link_options(${TARGET} PRIVATE --map=${TARGET}.map)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_options(${TARGET} PRIVATE -T${LINKER_SCRIPT})
        target_link_options(${TARGET} PRIVATE -Wl,-Map=${TARGET}.map)
        set_target_properties(${TARGET} PROPERTIES SUFFIX ".elf") 

    else()
        message(FATAL_ERROR "Unknown CMAKE_C_COMPILER_ID ${CMAKE_C_COMPILER_ID}")
    endif()
endfunction()

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()


# CMake function with automatic percentage calculation
function(add_arm_post_build_with_usage TARGET FLASH_SIZE_KB RAM_SIZE_KB)

    find_program(ARM_SIZE arm-none-eabi-size REQUIRED)
    find_program(ARM_OBJDUMP arm-none-eabi-objdump REQUIRED)
    find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)

    set(HEX_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.hex)
    set(BIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.bin)

    # Script that calculates the %
    set(SIZE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/size_report.cmake)

    file(WRITE ${SIZE_SCRIPT} "
if(NOT DEFINED ELF_FILE)
    message(FATAL_ERROR \"ELF_FILE not defined\")
endif()

execute_process(
    COMMAND ${ARM_SIZE} --format=berkeley \${ELF_FILE}
    OUTPUT_VARIABLE SIZE_OUTPUT
)

string(REGEX MATCH \"[0-9]+[ \\t]+[0-9]+[ \\t]+[0-9]+\" SIZE_LINE \${SIZE_OUTPUT})
string(REGEX MATCHALL \"[0-9]+\" SIZE_VALUES \${SIZE_LINE})

list(GET SIZE_VALUES 0 TEXT)
list(GET SIZE_VALUES 1 DATA)
list(GET SIZE_VALUES 2 BSS)

math(EXPR FLASH_USED \"\${TEXT} + \${DATA}\")
math(EXPR RAM_USED \"\${DATA} + \${BSS}\")

math(EXPR FLASH_TOTAL \"${FLASH_SIZE_KB} * 1024\")
math(EXPR RAM_TOTAL \"${RAM_SIZE_KB} * 1024\")

math(EXPR FLASH_PERCENT \"100 * \${FLASH_USED} / \${FLASH_TOTAL}\")
math(EXPR RAM_PERCENT \"100 * \${RAM_USED} / \${RAM_TOTAL}\")

message(\"\")
message(\"=========== STM32 Memory Usage ===========\")
message(\"Flash: \${FLASH_USED} / \${FLASH_TOTAL} bytes (\${FLASH_PERCENT}%)\")
message(\"RAM  : \${RAM_USED} / \${RAM_TOTAL} bytes (\${RAM_PERCENT}%)\")
message(\"===========================================\")
message(\"\")
")



    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${ARM_SIZE} --format=berkeley $<TARGET_FILE:${TARGET}>
        COMMAND ${ARM_SIZE} --format=SysV $<TARGET_FILE:${TARGET}>
        COMMAND ${ARM_OBJDUMP} -h $<TARGET_FILE:${TARGET}>
        COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET}> ${HEX_FILE}
        COMMAND ${ARM_OBJCOPY} -O binary $<TARGET_FILE:${TARGET}> ${BIN_FILE}
        COMMAND ${CMAKE_COMMAND}
                -DELF_FILE=$<TARGET_FILE:${TARGET}>
                -P ${SIZE_SCRIPT}
        COMMENT "Generating HEX/BIN and computing STM32U5 memory usage"
    )

endfunction()

