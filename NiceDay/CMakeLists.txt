cmake_minimum_required(VERSION 3.16.1)
project(NiceDay)

LIST( APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake/modules
)






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
        "vendor/stb_image/include/*.cpp"
        "vendor/imguifiledialog/*.cpp"
        #This is not required, imgui_build.cpp builds these
        #"vendor/imguidocking/examples/imgui_impl_glfw.cpp"
        #"vendor/imguidocking/examples/imgui_impl_opengl3.cpp"
        )
	

# Target creation
    add_library(NiceDay STATIC ${SOURCES})


# Include files
    target_include_directories(NiceDay PUBLIC 
        "src"
        "vendor/stb_image/include"
        "vendor/glm"
        "vendor/imguifiledialog"
        "vendor/sol2/include"
        "vendor/json/include"
        "vendor/entt/include"
        "vendor/rapidxml/include"

        #MONO_BRANCH
        #"C:/Program Files/Mono/include/mono-2.0"

    )


# Precompiled Header
    target_precompile_headers(NiceDay PRIVATE 
        "src/ndpch.h"
        )


# DEFINES
    target_compile_definitions(NiceDay PUBLIC 
        "_CRT_SECURE_NO_WARNINGS"
        #"LUA_BINARIES"
        "GLM_ENABLE_EXPERIMENTAL"
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

    # == ogg
    set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE BOOL "" FORCE)
    
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    add_my_lib(ogg)
    set(BUILD_TESTING ON CACHE BOOL "" FORCE)
    #add_custom_target(Ogg::ogg DEPENDS ogg)

    # == glm (no need to subdir (header-only))
    #set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
    #add_my_lib(glm glm_static)
    #target_include_directories(NiceDay PUBLIC "vendor/glm") 

    # == GLFW
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_UNINSTALL OFF CACHE BOOL "" FORCE)
    add_my_lib(glfw)


    # == vorbis
    modifyVorbisLibCmakeFile()
    nd_disable_find_package() #disable finding ogg package because its already added as subdirectory
    add_my_lib(vorbis vorbis vorbisfile vorbisenc)#directory called vorbis and three targets
    nd_enable_find_package()
    
    # == portadio
    # set(PA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    # set(PA_USE_ASIO OFF CACHE BOOL "" FORCE)
    # set(PA_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
    set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_my_lib(portaudio portaudio)
  
    # == assimp    
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    
    #enabling formats section:
    function(disableAssFormat)
        foreach(forma IN ITEMS ${ARGV})
            set(ASSIMP_BUILD_${forma}_EXPORTER OFF CACHE BOOL "" FORCE)
            set(ASSIMP_BUILD_${forma}_IMPORTER OFF CACHE BOOL "" FORCE)
        endforeach()
    endfunction()
    function(enableAssFormat format)
        foreach(forma IN ITEMS ${ARGV})
            set(ASSIMP_BUILD_${forma}_EXPORTER ON CACHE BOOL "" FORCE)
            set(ASSIMP_BUILD_${forma}_IMPORTER ON CACHE BOOL "" FORCE)
        endforeach()
    endfunction()
    
    disableAssFormat(ASSXML ASSJSON 3DS AC CSM FBX AMF ASE ASSBIN B3D BVH COLLADA DXF HMP IRRMESH IRR LWO LWS MD2 MD3 MD5 MDC MDL NFF NDO OFF OBJ OGRE OPENGEX PLY MS3D COB BLEND IFC XGL Q3D Q3BSP RAW SIB SMD STL TERRAGEN 3D X X3D GLTF 3MF MMD STEP)
    #enabled ass formats
    enableAssFormat(
        COLLADA
        FBX
        OBJ
        )  
    add_my_lib(assimp)
    #very important to change cxx_standard otherwise standard is 17 where certain things are deprecated and treated as error
    set_property(TARGET assimp PROPERTY CXX_STANDARD 14) 
    #set_target_properties(IrrXML PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    #set_target_properties(assimp_cmd PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    #set_target_properties(zlib PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    set_target_properties(zlibstatic PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    #set_target_properties(uninstall PROPERTIES USE_FOLDERS ON FOLDER "vendor")
    
    # == spdlog (no need to subdir (header-only))
    #add_subdirectory("vendor/spdlog")
    #list(APPEND EXTRA_LIBS spdlog_headers_for_ide)
    target_include_directories(NiceDay PUBLIC "vendor/spdlog/include") 


    # == nlohmann_json (no need to subdir (header-only))
    # set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    # add_subdirectory("vendor/nlohmann_json")
    target_include_directories(NiceDay PUBLIC
       "vendor/json/single_include"
    )
    target_compile_definitions(NiceDay PUBLIC GLFW_INCLUDE_NONE)
    target_compile_options(NiceDay PUBLIC /bigobj)


    #obtain mono installation
    SET(MONO_FIND_REQUIRED ON CACHE BOOL "" FORCE)
    SET(MONO_FIND_QUIETLY OFF CACHE BOOL "" FORCE)
    SET(MONO_ONLY_LIBRARIES_REQUIRED ON CACHE BOOL "" FORCE)
    SET(MONO_ROOT "C:/Program Files/Mono")
    #FIND_PACKAGE( Mono )
    #target_link_directories(NiceDay PUBLIC ${MONO_LIBRARY_DIRS})
    #TARGET_LINK_LIBRARIES(NiceDay PUBLIC ${MONO_LIBRARIES})


    #obtain dotnet
    set(ENV{DOTNET_ROOT} "C:/Program Files/dotnet")

    FIND_PACKAGE( Dotnet )

    message("FOund directory include " ${DOTNET_INCLUDE_DIRS})
    
    target_link_directories(NiceDay PUBLIC ${DOTNET_INCLUDE_DIRS})
    target_include_directories(NiceDay PUBLIC ${DOTNET_INCLUDE_DIRS})
    target_link_libraries(NiceDay PUBLIC ${DOTNET_IJWHOST_LIB} ${DOTNET_NETHOST_LIB} ${DOTNET_LIBNETHOST_LIB})

    
