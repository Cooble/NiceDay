cmake_minimum_required(VERSION 3.16.1)
project(TestNiceDay)

# Source files
    file(GLOB_RECURSE SOURCES 
        "src/*.cpp" 
        "src/*.hpp" 
        "src/*.h" 
        )


# Target creation
    add_executable(TestNiceDay ${SOURCES})
    add_test(MainTest TestNiceDay)
    #set_tests_properties(MainTest PROPERTIES WILL_FAIL FALSE)


# Include files
    target_include_directories(TestNiceDay PUBLIC 
        "src"
    )


# Dependencies
    target_link_libraries(TestNiceDay PUBLIC NiceDay)
    target_link_libraries(TestNiceDay PUBLIC Managed)
