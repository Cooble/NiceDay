 cmake_policy(VERSION 3.16.1)
 #needs TT_TARGET_DIR variable set to location of .exe
 #finds and copies mono dll to TT_TARGET_DIR
 #TT_SAUCE_DIR - root src

if(NOT EXISTS "${TT_SOURCE_DIR}")
    message(FATAL_ERROR "TT_SOURCE_DIR NOT SET!")
 else()
    message("Copying ManagedCore/ND.runtimeconfig.json --> ${TT_TARGET_DIR}")
    file(COPY "${TT_SOURCE_DIR}/ManagedCore/ND.runtimeconfig.json" DESTINATION "${TT_TARGET_DIR}/")
endif()

