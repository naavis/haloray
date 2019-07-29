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

set(RESOURCE_FILES resources/haloray.qrc)

add_library(halorayLib STATIC ${HALORAY_SOURCES} ${RESOURCE_FILES})
target_link_libraries(halorayLib Qt5::Core Qt5::Widgets Qt5::Gui)

IF (WIN32)
    add_executable(haloray WIN32 main.cpp resources/haloray.rc)
ELSE()
    add_executable(haloray main.cpp)
ENDIF()


target_link_libraries(haloray Qt5::Core Qt5::Widgets Qt5::Gui halorayLib)
