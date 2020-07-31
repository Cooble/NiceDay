cmake_minimum_required(VERSION 3.16.1)
project(Sandbox)

# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        "../res/registry/blocks/block_ids.h"
        "../res/registry/blocks/wall_ids.h"
        )


# Target creation
    add_executable(Sandbox ${SOURCES})


# Include files
    target_include_directories(Sandbox PUBLIC 
        "src"
        "../res/registry/headers"
    )

# Precompiled Header
#    target_precompile_headers(NiceDay PRIVATE 
#        "src/ndpch.h"
#        )


# DEFINES
#    target_compile_definitions(NiceDay PUBLIC 
#        "_CRT_SECURE_NO_WARNINGS"
#        #"LUA_BINARIES"
#        "$<$<PLATFORM_ID:Windows>:ND_PLATFORM_WINDOWS>"
#        "$<$<PLATFORM_ID:Linux>:ND_PLATFORM_LINUX>"
#        )


# Dependencies
    target_link_libraries(Sandbox PUBLIC NiceDay)
    target_link_libraries(Sandbox PUBLIC Managed)
#   macro(add_my_lib name)
#       add_subdirectory("vendor/${name}")
#       list(APPEND EXTRA_LIBS ${name})
#   endmacro()
#add_custom_command(TARGET Runner POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
#        "${PROJECT_BINARY_DIR}/vendor/Knihovna/$<CONFIG>/Knihovna.dll"
#        $<TARGET_FILE_DIR:Runner>)
execute_process(COMMAND cmake -P ${CMAKE_CURRENT_SOURCE_DIR}/../BuildIDHeader.cmake)

add_custom_target(
    BuildIdsTarget
    COMMAND cmake -P ${CMAKE_CURRENT_SOURCE_DIR}/../BuildIDHeader.cmake
)
add_dependencies(Sandbox BuildIdsTarget)