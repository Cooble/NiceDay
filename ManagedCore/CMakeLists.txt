cmake_minimum_required(VERSION 3.16.1)
project(ManagedCore LANGUAGES CSharp)
include(CSharpUtilities)

LIST( APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/modules
)
# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cs" 
        )
	

# Target creation
    add_library(ManagedCore SHARED ${SOURCES})
    #set_property(TARGET ManagedCore PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "net8.0")
	set_target_properties(ManagedCore PROPERTIES 
    DOTNET_SDK "Microsoft.NET.Sdk"
    DOTNET_TARGET_FRAMEWORK "net8.0")

    target_compile_options(ManagedCore PRIVATE "/langversion:12" "/unsafe")
