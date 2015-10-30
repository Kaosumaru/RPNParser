cmake_minimum_required (VERSION 2.8)
set(CMAKE_CXX_STANDARD 14)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

macro (add_folder name)
add_subdirectory(${name})
string(REPLACE "/" "\\" filtername ${name})
source_group(${filtername} FILES ${files})
endmacro()

project (RPN)


if (MSVC)
add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4005 /wd4996 /nologo" )
endif(MSVC)

##folders
add_folder(RPN)

set(ASMJIT_EMBED TRUE)
add_definitions(-DASMJIT_STATIC)
set(ASMJIT_DIR ${PROJECT_SOURCE_DIR}/../asmjit/ )
include("${ASMJIT_DIR}/CMakeLists.txt")

set(RPN_Includes ${PROJECT_SOURCE_DIR}/src/ ${PROJECT_SOURCE_DIR} ${ASMJIT_DIR}/src/ PARENT_SCOPE)
set(RPN_Deps ${ASMJIT_DEPS} PARENT_SCOPE)

add_library(RPN ${SRCS} ${ASMJIT_SRC})

##dependencies

##find_package(allegro REQUIRED)
##message(STATUS "Using SDL, include: ${SDL_INCLUDE_DIR}")
