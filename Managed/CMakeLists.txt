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
set_property(TARGET Managed PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5")
target_compile_options(Managed PRIVATE "/langversion:4")

# Include files
#   target_include_directories(NiceDay PUBLIC 
#       "src"
#       #MONO_BRANCH
#       "C:/Program Files/Mono/include/mono-2.0"
#
#   )
