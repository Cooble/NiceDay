 cmake_policy(VERSION 3.16.1)
 message("Generating BlockIDHeader id_header")
    function(nd_add_id_entries OUT_FIL NAMAE FILE_NAMAE OUTI)
        SET(OUTA "${OUT_FIL}")
        STRING(APPEND OUTA "\n\n//=======================${NAMAE} IDs=======================\n")
        file(READ "${FILE_NAMAE}" contents)
        
        STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
        STRING(REGEX REPLACE "\n" ";" contents "${contents}")
        foreach(ITEMEE IN ITEMS ${contents})
            STRING(REGEX MATCH "#" MATCHESSS "${ITEMEE}")
            if(NOT MATCHESSS)
                    STRING(REGEX REPLACE "\t" " " ITEMEE "${ITEMEE}")
                    STRING(REGEX REPLACE "  " " " ITEMEE "${ITEMEE}")
                    STRING(REGEX REPLACE "  " " " ITEMEE "${ITEMEE}")
                    STRING(REGEX REPLACE " " ";" SEXY_LIST ${ITEMEE})

                    LIST(GET SEXY_LIST 0 SEXY_INDEX)
                    LIST(GET SEXY_LIST 1 SEXY_NAME)
                
                    STRING(TOUPPER "${SEXY_NAME}" SEXY_NAME)
                    STRING(APPEND OUTA "constexpr int ${NAMAE}_${SEXY_NAME} = ${SEXY_INDEX};\n")
            endif()
        endforeach()
        SET(${OUTI} "${OUTA}" PARENT_SCOPE) 
    endfunction()

SET(OUT_FIL "#pragma once\n#include \"ndpch.h\"\n\n// This file is autogenerated by game (CMake)\n//		created from res/registry files\n// Any changes to this file will NOT have any effect!\n")       
nd_add_id_entries("${OUT_FIL}" "BLOCK" "${CMAKE_CURRENT_LIST_DIR}/res/registry/blocks/blocks.ids" OUT_FIL)
nd_add_id_entries("${OUT_FIL}" "WALL" "${CMAKE_CURRENT_LIST_DIR}/res/registry/blocks/walls.ids" OUT_FIL)
nd_add_id_entries("${OUT_FIL}" "BLOCK_FLAG" "${CMAKE_CURRENT_LIST_DIR}/res/registry/blocks/flags.ids" OUT_FIL)

file(READ "${CMAKE_CURRENT_LIST_DIR}/res/registry/headers/block_ids.h" original_content)
if(NOT "${original_content}" STREQUAL "${OUT_FIL}")
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/res/registry/headers/block_ids.h" "${OUT_FIL}")
    message("BlockIDHeader generated in: ${CMAKE_CURRENT_LIST_DIR}/res/registry/headers/block_ids.h")
else()
    message("Omitting BlockIDHeader -> NO CHANGE")
endif()
