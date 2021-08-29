add_executable(vstouch)
set_target_properties(vstouch PROPERTIES OUTPUT_NAME "vstouch.exe" PREFIX "" SUFFIX "")
target_sources(vstouch PRIVATE
  main.c
)
target_link_directories(vstouch PRIVATE
  vslib
)
target_include_directories(vstouch PRIVATE
  vslib
)
target_link_libraries(vstouch PRIVATE
  vslib.lib
)
file(COPY vslib/vslib.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(targets vstouch)
foreach(target ${targets})

if (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR
    ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"))

    target_compile_definitions(${target} PRIVATE
        _WIN32_WINNT=0x0601
        $<$<CONFIG:Release>:NDEBUG=1>
    )
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -pedantic-errors
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O3>
    )
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Release>:-s>
        -Wl,--kill-at
    )

elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")

    target_compile_definitions(${target} INTERFACE
        _WIN32_WINNT=0x0601
    )
    target_compile_options(${target} INTERFACE
        -W4
    )

endif()

endforeach(target)
