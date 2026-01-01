cmake_minimum_required(VERSION 3.16.1)
project(Terrain)

#if(MSVC)
#add_compile_options(/arch:AVX512)
#endif()

find_package(OpenCL REQUIRED)


# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        )


# Target creation
    add_executable(Terrain ${SOURCES})

    #enable openmp with compiler flags
    if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /openmp)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -fopenmp)
    endif()
   



# Include files
    #target_include_directories(Terrain PUBLIC 
    #    "src"
    #    ${OpenCL_INCLUDE_DIRS}
    #)
 target_include_directories(Terrain 
    PRIVATE "src"
    SYSTEM PRIVATE ${OpenCL_INCLUDE_DIRS}
)
message(STATUS "OpenCL Include Dirs: ${OpenCL_INCLUDE_DIRS}")



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
	target_link_libraries(Terrain PUBLIC OpenCL::OpenCL)

# Enable AVX512 if set
target_compile_options(Terrain PRIVATE
    # On MSVC if enabled, add /arch:AVX512
    $<$<AND:$<BOOL:${ENABLE_AVX512}>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX512>
    # On non-MSVC compilers if enabled, add -march=native
    $<$<AND:$<BOOL:${ENABLE_AVX512}>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-march=native>
)



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
