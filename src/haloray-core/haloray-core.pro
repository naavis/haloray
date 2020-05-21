TARGET = HaloRayCore
TEMPLATE = lib

QT += core gui widgets
CONFIG += c++17 static
win32:CONFIG += windows

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    gui/addCrystalPopulationButton.h \
    gui/crystalModel.h \
    gui/crystalSettingsWidget.h \
    gui/generalSettingsWidget.h \
    gui/mainWindow.h \
    gui/openGLWidget.h \
    gui/renderButton.h \
    gui/sliderSpinBox.h \
    gui/viewSettingsWidget.h \
    opengl/texture.h \
    opengl/textureRenderer.h \
    simulation/camera.h \
    simulation/crystalPopulation.h \
    simulation/crystalPopulationRepository.h \
    simulation/lightSource.h \
    simulation/simulationEngine.h

SOURCES += \
    gui/addCrystalPopulationButton.cpp \
    gui/crystalModel.cpp \
    gui/crystalSettingsWidget.cpp \
    gui/generalSettingsWidget.cpp \
    gui/mainWindow.cpp \
    gui/openGLWidget.cpp \
    gui/renderButton.cpp \
    gui/sliderSpinBox.cpp \
    gui/viewSettingsWidget.cpp \
    opengl/texture.cpp \
    opengl/textureRenderer.cpp \
    simulation/camera.cpp \
    simulation/crystalPopulation.cpp \
    simulation/crystalPopulationRepository.cpp \
    simulation/lightSource.cpp \
    simulation/simulationEngine.cpp

RESOURCES = \
    resources/haloray.qrc
