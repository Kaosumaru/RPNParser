cmake_minimum_required (VERSION 2.8)
set(CMAKE_CXX_STANDARD 14)

project (Tests)

if (MSVC)
add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4005 /wd4996 /nologo" )
endif(MSVC)

add_definitions(-DRPN_OPTIMIZE_0) #disable constant optimization, so 2+2 would be a tree with 3 tokens, not 1

add_subdirectory(../RPN "${CMAKE_CURRENT_BINARY_DIR}/RPN")
include_directories ("${RPN_Includes}")

add_definitions(-DASMJIT_STATIC)
add_executable (Tests main.cpp)
target_Link_Libraries(Tests ${RPN_Deps})
target_link_libraries (Tests RPN)
