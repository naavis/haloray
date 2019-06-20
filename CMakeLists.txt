cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(HaloRay)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(src)
