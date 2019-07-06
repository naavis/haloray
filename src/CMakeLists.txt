# find includes in the corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run Qt RCC for resource files
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS REQUIRED Core Widgets Gui)

set(HALORAY_SOURCES
    gui/mainWindow.cpp
    gui/openGLWidget.cpp
    gui/generalSettingsWidget.cpp
    gui/crystalSettingsWidget.cpp
    gui/viewSettingsWidget.cpp
    gui/sliderSpinBox.cpp
    gui/renderButton.cpp
    gui/crystalModel.cpp
    gui/addCrystalPopulationButton.cpp
    simulation/simulationEngine.cpp
    simulation/camera.cpp
    simulation/lightSource.cpp
    simulation/crystalPopulation.cpp
    simulation/crystalPopulationRepository.cpp
    opengl/texture.cpp
    opengl/textureRenderer.cpp
)

set(RESOURCE_FILES resources/haloray.qrc resources/haloray.rc)

add_library(halorayLib STATIC ${HALORAY_SOURCES} ${RESOURCE_FILES})
target_link_libraries(halorayLib Qt5::Core Qt5::Widgets Qt5::Gui)

IF (WIN32)
    add_executable(haloray WIN32 main.cpp)
ELSE()
    add_executable(haloray main.cpp)
ENDIF()


target_link_libraries(haloray Qt5::Core Qt5::Widgets Qt5::Gui halorayLib)
