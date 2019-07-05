configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

# find includes in the corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS REQUIRED Core Widgets Gui)

set(HALORAY_SOURCES
    main.cpp
    gui/mainWindow.cpp
    gui/openGLWidget.cpp
    gui/generalSettingsWidget.cpp
    gui/crystalSettingsWidget.cpp
    gui/viewSettingsWidget.cpp
    gui/sliderSpinBox.cpp
    gui/renderButton.cpp
    simulation/simulationEngine.cpp
    opengl/texture.cpp
    opengl/textureRenderer.cpp
)

IF (WIN32)
    add_executable(haloray WIN32 ${HALORAY_SOURCES})
ELSE()
    add_executable(haloray ${HALORAY_SOURCES})
ENDIF()

target_link_libraries(haloray Qt5::Core Qt5::Widgets Qt5::Gui)
