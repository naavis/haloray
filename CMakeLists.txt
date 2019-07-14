cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(HaloRay)
include(Dart)

# Optionally set the git branch in AppVeyor
option(GIT_BRANCH "Current git branch" HEAD)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
add_definitions("-DGIT_BRANCH=${GIT_BRANCH}")

if (HALORAY_VERSION)
    # Strip running build number from version
    string(REGEX REPLACE "-.+$" "" HALORAY_VERSION_NO_BUILD_NUMBER ${HALORAY_VERSION})
    add_definitions(-DHALORAY_VERSION=${HALORAY_VERSION_NO_BUILD_NUMBER})
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# find includes in the corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run Qt RCC for resource files
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS REQUIRED Core Widgets Gui Test)

add_subdirectory(src)
add_subdirectory(tests)
