cmake_minimum_required (VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)

project (Benchmarks)

if (MSVC)
add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4005 /wd4996 /nologo" )
endif(MSVC)

add_subdirectory(../RPN "${CMAKE_CURRENT_BINARY_DIR}/RPN")
include_directories ("${RPN_Includes}")

add_definitions(-DASMJIT_STATIC)
add_executable (Benchmarks main.cpp)
target_Link_Libraries(Benchmarks ${RPN_Deps})
target_link_libraries (Benchmarks RPN)
