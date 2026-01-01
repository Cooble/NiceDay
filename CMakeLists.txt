#Base CMAKE
cmake_minimum_required(VERSION 3.16.1)


project("NiceDaySolution")


set(CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>"   # /MT in Release, /MTd in Debug
    CACHE STRING "Use static VC++ runtime (/MT or /MTd)" FORCE)

option(ND_ENABLE_DOTNET "Build with C# scripting support" OFF)   # default = ON
option(ENABLE_AVX512 "Enable AVX512 instruction set optimizations" OFF)
message(STATUS "AVX512 ENABLED: ${ENABLE_AVX512}")
add_compile_options(
    # On MSVC if enabled, add /arch:AVX512
    $<$<AND:$<BOOL:${ENABLE_AVX512}>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX512>
    # On non-MSVC compilers if enabled, add -march=native
    $<$<AND:$<BOOL:${ENABLE_AVX512}>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-march=native>
)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

LIST( APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/modules
)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # hack to prevent CTest added targets

MARK_AS_ADVANCED(MULTITHREADED_BUILD)
set(MULTITHREADED_BUILD 32 CACHE STRING "How many threads are used to build the project")
if(${CMAKE_GENERATOR} MATCHES "Unix Makefiles")
       message(STATUS ${CMAKE_BUILD_TOOL})
       set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j${MULTITHREADED_BUILD}")
       message(STATUS "Added arguments to CMAKE_BUILD_TOOL: ${CMAKE_MAKE_PROGRAM}")
elseif(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #paralell 
      message(STATUS "Added parallel build arguments to CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
endif()

# add Configurations Debug Release and Dist
if(CMAKE_CONFIGURATION_TYPES)
   list(APPEND CMAKE_CONFIGURATION_TYPES Dist)
   list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
   list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel)
   #list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel RelWithDebInfo)
   message("    Available build types: ${CMAKE_CONFIGURATION_TYPES}")
   
   #add flags to config dist
   set(CMAKE_EXE_LINKER_FLAGS_DIST ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
   set(CMAKE_SHARED_LINKER_FLAGS_DIST ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
  
else()
   message("    XXX custom build types are not allowed...")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()


#Global DEFINES based on Config
set(DEBUG_COMP_DEFINITIONS 
    ND_DEBUG
    ND_CONFIG="ND_DEBUG"
)

set(RELEASE_COMP_DEFINITIONS 
    ND_RELEASE
    ND_CONFIG="ND_RELEASE"
)

set(DIST_COMP_DEFINITIONS 
    ND_DIST
    ND_CONFIG="ND_DIST"
)

add_compile_definitions(
    "$<$<CONFIG:Debug>:${DEBUG_COMP_DEFINITIONS}>" 
    "$<$<CONFIG:Dist>:${DIST_COMP_DEFINITIONS}>" 
    "$<$<CONFIG:RelWithDebInfo>:${RELEASE_COMP_DEFINITIONS}>" 
    "$<$<CONFIG:Release>:${RELEASE_COMP_DEFINITIONS}>")

#set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_SYSTEM_PREFIX_PATH} "./")
# Subdirs
add_subdirectory(NiceDay)

if(ND_ENABLE_DOTNET)
    add_subdirectory(ManagedCore)
    add_subdirectory(Managed)
endif()
add_subdirectory(Sandbox)
add_subdirectory(SandboxTest)
add_subdirectory(Terrain)
enable_testing()
add_subdirectory(TestNiceDay)

