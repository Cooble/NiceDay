cmake_minimum_required(VERSION 3.16.1)
project(TestNiceDay)

# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        )


# Target creation
    #set_tests_properties(MainTest PROPERTIES WILL_FAIL FALSE)
    add_executable(TestNiceDay ${SOURCES})
    macro(add_my_test TESTO)
      add_test(${TESTO} TestNiceDay ${TESTO})
    endmacro()

    add_my_test(NDUtilTest)
    add_my_test(monoTest)
 


# Include files
    target_include_directories(TestNiceDay PUBLIC 
        "src"
    )


# Dependencies
    target_link_libraries(TestNiceDay PUBLIC NiceDay)
    target_link_libraries(TestNiceDay PUBLIC Managed)
