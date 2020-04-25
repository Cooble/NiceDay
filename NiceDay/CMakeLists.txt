cmake_minimum_required(VERSION 3.16.1)
project(NiceDay)


#this macro will disable package finding
    macro (nd_disable_find_package)
        set(ND_DISABLE_FIND_PACKAGE ON CACHE STRING "Disabling find_package macro" FORCE)
    endmacro()
    macro (nd_enable_find_package)
        set(ND_DISABLE_FIND_PACKAGE OFF CACHE STRING "Disabling find_package macro" FORCE)
    endmacro()

    macro(find_package)
        if(NOT ND_DISABLE_FIND_PACKAGE)
            _find_package(${ARGV})
        endif()
    endmacro()

# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        #"vendor/glm/**.hpp"
        "vendor/json/include/**.hpp"
        "vendor/luabridge/**"
        "vendor/stb_image/*.h"
        "vendor/stb_image/*.cpp"
        "vendor/imguifiledialog/*.h"
        "vendor/imguifiledialog/*.cpp"
        "vendor/imgui/imgui/examples/imgui_impl_glfw.cpp"
        "vendor/imgui/imgui/examples/imgui_impl_opengl3.cpp"
        )
	

# Target creation
    add_library(NiceDay STATIC ${SOURCES})


# Include files
    target_include_directories(NiceDay PUBLIC 
        "src"
        "vendor/stb_image"
        "vendor/imguifiledialog"
        "vendor/luabridge"
       
    )


# Precompiled Header
    target_precompile_headers(NiceDay PRIVATE 
        "src/ndpch.h"
        )


# DEFINES
    target_compile_definitions(NiceDay PUBLIC 
        "_CRT_SECURE_NO_WARNINGS"
        #"LUA_BINARIES"
        "$<$<PLATFORM_ID:Windows>:ND_PLATFORM_WINDOWS>"
        "$<$<PLATFORM_ID:Linux>:ND_PLATFORM_LINUX>"
        )


# Dependencies
     # links to Niceday and sets props to be in folder called vendor
    function(link_my_lib)
        target_link_libraries(NiceDay PUBLIC ${ARGV})
        set_target_properties(${ARGV} PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    endfunction()

    # adds subdir and links the lib to niceday
    function(add_my_lib folder_name) #if name of lib is same as directory
        message("Adding subdirectory: : ${folder_name}")
        add_subdirectory("vendor/${folder_name}")
        if (ARGC EQUAL 1)
            link_my_lib(${folder_name})
        else()
            link_my_lib(${ARGN})
        endif()
    endfunction()
    function(modifyVorbisLibCmakeFile)
        message("Changing/HACKING! vendor/vorbis/lib/CMakeLists.txt to allow ogg subdir import")
        message("   -> replacing string \"Ogg::ogg\" for \"ogg\"")
        file(READ "vendor/vorbis/lib/CMakeLists.txt" TEXTO)
        string(REPLACE "PUBLIC Ogg::ogg" "PUBLIC ogg" TEXTO2 ${TEXTO})
        file(WRITE "vendor/vorbis/lib/CMakeLists.txt" ${TEXTO2})
    endfunction()
    #add_custom_command(TARGET Runner POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    #        "${PROJECT_BINARY_DIR}/vendor/Knihovna/$<CONFIG>/Knihovna.dll"
    #        $<TARGET_FILE_DIR:Runner>)
    
    # simple libs (no config needed and directory is same as target name)
    add_my_lib(glad)
    add_my_lib(imgui)
    add_my_lib(lua)

    # ogg
    set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    add_my_lib(ogg)
    #add_custom_target(Ogg::ogg DEPENDS ogg)

    # glm (no need to subdir (header-only))
    #set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
    #add_my_lib(glm glm_static)
    target_include_directories(NiceDay PUBLIC "vendor/glm") 

    # GLFW
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_UNINSTALL OFF CACHE BOOL "" FORCE)
    add_my_lib(glfw)


    # vorbis
    modifyVorbisLibCmakeFile()
    nd_disable_find_package() #disable finding ogg package because its already added as subdirectory
    add_my_lib(vorbis vorbis vorbisfile vorbisenc)#directory called vorbis and three targets
    nd_enable_find_package()
    
    # portadio
    set(PA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(PA_USE_ASIO OFF CACHE BOOL "" FORCE)
    set(PA_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
    add_my_lib(portaudio portaudio_static)
    
    # spdlog (no need to subdir (header-only))
    #add_subdirectory("vendor/spdlog")
    #list(APPEND EXTRA_LIBS spdlog_headers_for_ide)
    target_include_directories(NiceDay PUBLIC "vendor/spdlog/include") 


    # nlohmann_json (no need to subdir (header-only))
    # set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    # add_subdirectory("vendor/nlohmann_json")
    target_include_directories(NiceDay PUBLIC
       "vendor/json/include"
       "vendor/json/single_include"
    )
    target_compile_definitions(NiceDay PUBLIC GLFW_INCLUDE_NONE)
