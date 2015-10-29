cmake_minimum_required (VERSION 2.8)

project (Tests)

if (MSVC)
add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4005 /wd4996 /nologo" )
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)

add_subdirectory(../RPN "${CMAKE_CURRENT_BINARY_DIR}/RPN")
include_directories ("${PROJECT_SOURCE_DIR}/../RPN")

#set(ASMJIT_EMBED TRUE)
add_subdirectory(../asmjit "${CMAKE_CURRENT_BINARY_DIR}/asmjit")
include_directories ("${PROJECT_SOURCE_DIR}/../asmjit/src/")

add_executable (Tests main.cpp)
target_link_libraries (Tests RPN asmjit)
