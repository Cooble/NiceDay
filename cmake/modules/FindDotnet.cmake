# FindDotnet.cmake
#
# This module locates the .NET Core SDK and runtime libraries for embedding the .NET runtime in C++ projects.
# It defines the following variables:
#
#   Dotnet_FOUND        - True if .NET SDK was found
#   DOTNET_SDK_DIR      - The root directory of the .NET SDK
#   DOTNET_CORECLR_LIB  - The coreclr library path
#   DOTNET_HOSTFXR_LIB  - The hostfxr library path
#   DOTNET_INCLUDE_DIRS - The include directory for hostfxr.h and other headers

if(NOT DEFINED ENV{DOTNET_ROOT})
    message(FATAL_ERROR "DOTNET_ROOT environment variable is not set. Please set DOTNET_ROOT to the root of your .NET SDK installation.")
endif()

set(DOTNET_SDK_DIR $ENV{DOTNET_ROOT})
message(STATUS "Found .NET SDK in ${DOTNET_SDK_DIR}")

# Platform detection (adjust to handle win-x86, linux-x64, osx, etc.)
if(WIN32)
    set(DOTNET_PACK "Microsoft.NETCore.App.Host.win-x64")
elseif(UNIX AND NOT APPLE)
    set(DOTNET_PACK "Microsoft.NETCore.App.Host.linux-x64")
elseif(APPLE)
    set(DOTNET_PACK "Microsoft.NETCore.App.Host.osx-x64")
endif()

# Specify the Major version of .NET Core you are using
set(DOTNET_MAJOR_VERSION "8.*") # dont forget to change this also in CopyMonoDll.cmake


# Get highest version starting with 
file(GLOB version_dirs "${DOTNET_SDK_DIR}/packs/${DOTNET_PACK}/${DOTNET_MAJOR_VERSION}")
set(highest_version "")
foreach(ver ${version_dirs})
    get_filename_component(file_name ${ver} NAME)

    # Normalize version to list of numbers
    string(REPLACE "." ";" version_parts ${file_name})
    list(GET version_parts 0 major)
    list(GET version_parts 1 minor)
    list(GET version_parts 2 patch)

    if(NOT DEFINED best_major)
        set(best_major ${major})
        set(best_minor ${minor})
        set(best_patch ${patch})
        set(highest_version ${file_name})
    else()
        if(minor GREATER best_minor OR (minor EQUAL best_minor AND patch GREATER best_patch))
            set(best_minor ${minor})
            set(best_patch ${patch})
            set(highest_version ${file_name})
        endif()
    endif()
endforeach()

message(STATUS "Highest version starting with ${DOTNET_MAJOR_VERSION}: ${highest_version}")
set(DOTNET_VERSION "${highest_version}")



# Define the path to the native runtime files (adjust the pack location and structure)
set(DOTNET_INCLUDE_DIRS "${DOTNET_SDK_DIR}/packs/${DOTNET_PACK}/${DOTNET_VERSION}/runtimes/win-x64/native")
if(NOT EXISTS "${DOTNET_INCLUDE_DIRS}/hostfxr.h")
    message(FATAL_ERROR "Could not find hostfxr.h in ${DOTNET_INCLUDE_DIRS}")
endif()
message(STATUS "Found .NET SDK headers in ${DOTNET_INCLUDE_DIRS}")


find_library(DOTNET_IJWHOST_LIB NAMES ijwhost PATHS "${DOTNET_INCLUDE_DIRS}")
find_library(DOTNET_NETHOST_LIB NAMES nethost PATHS "${DOTNET_INCLUDE_DIRS}")
find_library(DOTNET_LIBNETHOST_LIB NAMES nethost PATHS "${DOTNET_INCLUDE_DIRS}")


message(STATUS "Found ijwhost library: ${DOTNET_IJWHOST_LIB}")
message(STATUS "Found nethost library: ${DOTNET_NETHOST_LIB}")
message(STATUS "Found libnethost library: ${DOTNET_LIBNETHOST_LIB}")

# message error if any of the libraries are not found
if(NOT DOTNET_IJWHOST_LIB)
	message(FATAL_ERROR "Could not find ijwhost library")
endif()
if(NOT DOTNET_NETHOST_LIB)
    message(FATAL_ERROR "Could not find nethost library")
endif()
if(NOT DOTNET_LIBNETHOST_LIB)
    message(FATAL_ERROR "Could not find libnethost library")
endif()


# Set output variables
set(Dotnet_FOUND TRUE)
# set to parent scope
set(DOTNET_IJWHOST_LIB ${DOTNET_IJWHOST_LIB} PARENT_SCOPE)
set(DOTNET_NETHOST_LIB ${DOTNET_NETHOST_LIB} PARENT_SCOPE)
set(DOTNET_LIBNETHOST_LIB ${DOTNET_LIBNETHOST_LIB} PARENT_SCOPE)
set(DOTNET_INCLUDE_DIRS ${DOTNET_INCLUDE_DIRS} PARENT_SCOPE)
