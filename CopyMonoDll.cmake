 cmake_policy(VERSION 3.16.1)
 #needs TT_TARGET_DIR variable set to location of .exe
 #finds and copies mono dll to TT_TARGET_DIR
function(FindMon MONO_ROOT)
    set(PATHETIC_PATHS $ENV{PATH})
    LIST(APPEND PATHETIC_PATHS "C:\\Program Files")
    LIST(APPEND PATHETIC_PATHS "C:\\Program Files (x86)")
    set(FOUND OFF)
    set(OUT "")
    foreach(pathe IN ITEMS ${PATHETIC_PATHS})
        #message("Searching for mono ${pathe}")
         if("${pathe}" MATCHES "dotnet(\\|/|)$")
            message("Found dotnet ${pathe}")
            set(OUT "${pathe}")
            break()
        endif()
        
        #check children
        file(GLOB SUBDIRSS "${pathe}/*")
        FOREACH(subdir ${SUBDIRSS})
            if(IS_DIRECTORY ${subdir})
         #       message(" ---> ${subdir}")
               if("${subdir}" MATCHES "dotnet(\\|/|)$")
                    message("Found dotnet ${subdir}")
                    set(OUT "${subdir}")
                    set(FOUND ON)
                    break()
                endif()
            endif() 
        ENDFOREACH()
        if(FOUND)
           break()
        endif()
    endforeach()
    set(${MONO_ROOT} ${OUT} PARENT_SCOPE)
 endfunction()
 
FindMon(MONO_ROOT)
message("${MONO_ROOT}")
if(NOT EXISTS "${MONO_ROOT}")
    message(FATAL_ERROR "Cannot find Dotnet installation dir")
 else()


    # TODO this is blatantly copied from FindDotnet.cmake
    # Please refactor this

    # Specify the Major version of .NET Core you are using
    set(DOTNET_MAJOR_VERSION "8.*") # do not forget to change this also in FindDotnet.cmake

    # Platform detection (adjust to handle win-x86, linux-x64, osx, etc.)
    if(WIN32)
    set(DOTNET_PACK "Microsoft.NETCore.App.Host.win-x64")
    elseif(UNIX AND NOT APPLE)
    set(DOTNET_PACK "Microsoft.NETCore.App.Host.linux-x64")
    elseif(APPLE)
    set(DOTNET_PACK "Microsoft.NETCore.App.Host.osx-x64")
    endif()


    # Get highest version starting with 
    file(GLOB version_dirs "${MONO_ROOT}/packs/${DOTNET_PACK}/${DOTNET_MAJOR_VERSION}")
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



    message("Copying ${MONO_ROOT}/packs/Microsoft.NETCore.App.Host.win-x64/8.0.10/runtimes/win-x64/native/nethost.dll --> ${TT_TARGET_DIR}")
    #file(COPY "${MONO_ROOT}/bin/mono-2.0-sgen.dll" DESTINATION "${TT_TARGET_DIR}/")
    # //todo use relative path for copying nethost.dll
    file(COPY "${MONO_ROOT}/packs/${DOTNET_PACK}/${highest_version}/runtimes/win-x64/native/nethost.dll" DESTINATION "${TT_TARGET_DIR}/")
endif()

