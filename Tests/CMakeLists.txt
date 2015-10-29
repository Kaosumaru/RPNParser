cmake_minimum_required (VERSION 2.8)

project (Tests)

if (MSVC)
add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4005 /wd4996 /nologo" )
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)

add_subdirectory(../RPN "${CMAKE_CURRENT_BINARY_DIR}/RPN")
include_directories ("${PROJECT_SOURCE_DIR}/../RPN")

set(ASMJIT_EMBED TRUE)
add_Definitions(-DASMJIT_STATIC)
set(ASMJIT_DIR ${PROJECT_SOURCE_DIR}/../asmjit/ )
Include("${ASMJIT_DIR}/CMakeLists.txt")
include_directories ("${PROJECT_SOURCE_DIR}/src/")


add_executable (Tests main.cpp ${ASMJIT_SRC})
target_link_libraries (Tests RPN)

target_Link_Libraries(YourTarget ${ASMJIT_DEPS})
