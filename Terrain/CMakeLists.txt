cmake_minimum_required(VERSION 3.16.1)
project(Terrain)

# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        )


# Target creation
    add_executable(Terrain ${SOURCES})


# Include files
    target_include_directories(Terrain PUBLIC 
        "src"
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
    target_link_libraries(Terrain PUBLIC NiceDay)
if(ND_ENABLE_DOTNET)
    target_link_libraries(Terrain PUBLIC Managed)
    
    add_custom_command(TARGET Terrain 
	 POST_BUILD
     COMMAND  ${CMAKE_COMMAND} -DTT_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/CopyMonoDll.cmake
     COMMENT "copying mono dll to build dir..."
     )
    add_custom_command(TARGET Terrain 
	POST_BUILD
    COMMAND  ${CMAKE_COMMAND} -DTT_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} -DTT_SOURCE_DIR=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/CopyMonoConfig.cmake
    COMMENT "copying mono config to build dir..."
    )
endif()
