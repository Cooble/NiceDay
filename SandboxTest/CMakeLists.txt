cmake_minimum_required(VERSION 3.16.1)
project(SandboxTest)

# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        )


# Target creation
    add_executable(SandboxTest ${SOURCES})


# Include files
    target_include_directories(SandboxTest PUBLIC 
        "src"
    )
	add_custom_command(TARGET SandboxTest 
	POST_BUILD
    COMMAND  ${CMAKE_COMMAND} -DTT_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/CopyMonoDll.cmake
    COMMENT "copying mono dll to build dir..."
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
    target_link_libraries(SandboxTest PUBLIC NiceDay)
    target_link_libraries(SandboxTest PUBLIC Managed)
#   macro(add_my_lib name)
#       add_subdirectory("vendor/${name}")
#       list(APPEND EXTRA_LIBS ${name})
#   endmacro()
#add_custom_command(TARGET Runner POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
#        "${PROJECT_BINARY_DIR}/vendor/Knihovna/$<CONFIG>/Knihovna.dll"
#        $<TARGET_FILE_DIR:Runner>)
