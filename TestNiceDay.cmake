cmake_minimum_required (VERSION 3.1)

include("FindMono.cmake")
include("Utills.cmake")

message("Running NiceDay Tests")
PrintDir("${MONO_ROOT}" " " 2)

message("Copying dll")
file(COPY "C:/Program Files/Mono/bin/mono-2.0-sgen.dll" DESTINATION "${ROOT_DIRI}/MonoTesting/Release/")

PrintDir("${ROOT_DIRI}/MonoTesting/Release" " " 3)
 SET(PA "${ROOT_DIRI}/MonoTesting/Release/MonoTesting.exe")
 message("root dir is: ${PA}")
 execute_process(COMMAND "${PA}"
                RESULT_VARIABLE RES_VAR
                OUTPUT_VARIABLE OUT_VAR                
                ERROR_VARIABLE OUT_VAR)
message("Proccess exited with: ${RES_VAR}")
message("Output is: ${OUT_VAR}")

if(NOT RES_VAR STREQUAL "0")
    message(FATAL_ERROR "Tests failed")
else()
    message("Tests succeded")
endif()
