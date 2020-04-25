cmake_minimum_required(VERSION 3.16)
project(Lua)
file(GLOB_RECURSE SOURCES 
	"*.c" 
	"*.cpp" 
	"*.h" 
	"*.hpp" 
	)
file(GLOB_RECURSE EXCLUDED_SOURCES 
	"*lua.c" 
	"*luac.c" 
	)
	
list(REMOVE_ITEM SOURCES ${EXCLUDED_SOURCES})

add_library(lua STATIC ${SOURCES})

target_include_directories(lua PUBLIC "src/lua")