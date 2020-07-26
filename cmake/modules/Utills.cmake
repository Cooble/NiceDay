# Utils needed in project NICEDAY
cmake_minimum_required (VERSION 3.1)


# equivalent to "ls" but recursive
# example usage: PrintDir("${YOUR_FOLDER_PATH}" " " 3)
function(PrintDir PATH OFFSET MAX_DEEP)

set(DEEPNES ${MAX_DEEP})
MATH(EXPR DEEPNES "${DEEPNES}-1")

if("${DEEPNES}" STREQUAL "0")
message("${OFFSET}${PATH}/**")
return()
endif()
message("${OFFSET}${PATH}")
file(GLOB SUBDIRSS "${PATH}/*")
FOREACH(subdir ${SUBDIRSS})
            if(IS_DIRECTORY ${subdir})
              PrintDir(${subdir} "${OFFSET}     " ${DEEPNES})
            else()
                message("${OFFSET}     ${subdir}")
            endif() 
ENDFOREACH()

endfunction()