add_library(warnings_strict INTERFACE)

target_compile_options(warnings_strict INTERFACE
  -Wall
  -Wcast-align
  -Wcast-qual
  -Wdangling-else
  -Wdouble-promotion
  -Wenum-conversion
  -Werror
  -Wextra
  -Wfatal-errors
  -Wformat
  -Wimplicit-fallthrough
  -Wredundant-decls
  -Wshadow
  -Wswitch-enum
  -Wundef
  -Wuninitialized
  -Wunused-const-variable
  -Wunused-macros
  -Wunused-parameter
  -Wunused-variable
  -pedantic
)

add_library(warnings_relaxed INTERFACE)

target_compile_options(warnings_relaxed INTERFACE
  -Wall
  -Wextra
  -Wno-cast-align
  -Wno-unused-parameter
  -Wno-unused-function
  -Wno-unused-variable
  -Wno-unused-macros
  -Wno-redundant-decls
)

add_library(warnings_cpp INTERFACE)

target_compile_options(warnings_cpp INTERFACE
  -Wconversion
  -Wctor-dtor-privacy
  -Wextra-semi
  -Winvalid-constexpr
  -Wmismatched-tags
  -Wold-style-cast
  -Wsign-promo
)

function(apply_project_warnings target level)
    if(level STREQUAL "STRICT")
        target_link_libraries(${target} PRIVATE warnings_strict)
    elseif(level STREQUAL "RELAXED")
        target_link_libraries(${target} PRIVATE warnings_relaxed)
    elseif(level STREQUAL "CPP")
        target_link_libraries(${target} PRIVATE warnings_cpp)
    else()
        message(FATAL_ERROR
            "Unknown warning level '${level}' for target '${target}'"
        )
    endif()
endfunction()