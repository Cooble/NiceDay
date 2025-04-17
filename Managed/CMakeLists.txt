cmake_minimum_required(VERSION 3.16.1)
project(Managed LANGUAGES CSharp)
include(CSharpUtilities)

LIST( APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/modules
)
# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cs" 
        )
   

# Target creation
    add_library(Managed SHARED ${SOURCES})
    #target_include_directories(Managed "../ManagedCore/src")

    #set_property(TARGET ManagedCore PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "net8.0")
    set_target_properties(ManagedCore PROPERTIES 
    DOTNET_SDK "Microsoft.NET.Sdk"
    DOTNET_TARGET_FRAMEWORK "net8.0")

    target_compile_options(Managed PRIVATE "/langversion:12" "/unsafe")
    target_link_libraries(Managed ManagedCore)

# Include files
#   target_include_directories(NiceDay PUBLIC 
#       "src"
#       #MONO_BRANCH
#       "C:/Program Files/Mono/include/mono-2.0"
#
#   )
