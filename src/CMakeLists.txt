set (HaloRay_VERSION_MAJOR 1)
set (HaloRay_VERSION_MINOR 1)
set (HaloRay_VERSION_PATCH 0)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

# find includes in the corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS REQUIRED Core Widgets Gui)

add_executable(haloray
    main.cpp
    gui/mainWindow.ui
    gui/mainWindow.cpp
    simulation/simulationEngine.cpp
    opengl/utils.cpp
    opengl/texture.cpp
    opengl/shader.cpp
    opengl/textureRenderer.cpp
    opengl/program.cpp
    )

target_link_libraries(haloray ${CONAN_LIBS} Qt5::Core Qt5::Widgets Qt5::Gui)
