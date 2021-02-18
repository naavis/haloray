TARGET = HaloRayCore
TEMPLATE = lib

QT += core gui widgets
CONFIG += c++17 static
win32:CONFIG += windows

DEFINES += QT_DEPRECATED_WARNINGS

GIT_COMMIT_HASH=$$system(git log -1 --format=%h)

DEFINES += "GIT_COMMIT_HASH=\"$$GIT_COMMIT_HASH\""
GIT_BRANCH = $$(APPVEYOR_REPO_BRANCH)
isEmpty(GIT_BRANCH) {
    LOCAL_GIT_BRANCH=$$system(git rev-parse --abbrev-ref HEAD)
    DEFINES += "GIT_BRANCH=\"$$LOCAL_GIT_BRANCH\""
} else {
    DEFINES += "GIT_BRANCH=\"$$GIT_BRANCH\""
}

HALORAY_VERSION = $$(HALORAY_VERSION)
!isEmpty(HALORAY_VERSION) {
    DEFINES += "HALORAY_VERSION=\"$$HALORAY_VERSION\""
}

HEADERS += \
    gui/atmosphereSettingsWidget.h \
    gui/components/addCrystalPopulationButton.h \
    gui/components/collapsibleBox.h \
    gui/components/renderButton.h \
    gui/components/sliderSpinBox.h \
    gui/crystalPreview/crystalPreviewWindow.h \
    gui/crystalPreview/previewRenderArea.h \
    gui/crystalSettingsWidget.h \
    gui/generalSettingsWidget.h \
    gui/mainWindow.h \
    gui/massEditor.h \
    gui/models/crystalModel.h \
    gui/models/simulationStateModel.h \
    gui/openGLWidget.h \
    gui/stateSaver.h \
    gui/viewSettingsWidget.h \
    opengl/texture.h \
    opengl/textureRenderer.h \
    simulation/atmosphere.h \
    simulation/colorUtilities.h \
    simulation/hosekWilkie/ArHosekSkyModel.h \
    simulation/hosekWilkie/ArHosekSkyModelData_CIEXYZ.h \
    simulation/hosekWilkie/ArHosekSkyModelData_RGB.h \
    simulation/hosekWilkie/ArHosekSkyModelData_Spectral.h \
    simulation/camera.h \
    simulation/crystalPopulation.h \
    simulation/crystalPopulationRepository.h \
    simulation/lightSource.h \
    simulation/simulationEngine.h \
    simulation/skyModel.h \
    simulation/trigonometryUtilities.h

SOURCES += \
    gui/atmosphereSettingsWidget.cpp \
    gui/components/addCrystalPopulationButton.cpp \
    gui/components/collapsibleBox.cpp \
    gui/components/renderButton.cpp \
    gui/components/sliderSpinBox.cpp \
    gui/crystalPreview/crystalPreviewWindow.cpp \
    gui/crystalPreview/previewRenderArea.cpp \
    gui/crystalSettingsWidget.cpp \
    gui/generalSettingsWidget.cpp \
    gui/mainWindow.cpp \
    gui/massEditor.cpp \
    gui/models/crystalModel.cpp \
    gui/models/simulationStateModel.cpp \
    gui/openGLWidget.cpp \
    gui/stateSaver.cpp \
    gui/viewSettingsWidget.cpp \
    opengl/texture.cpp \
    opengl/textureRenderer.cpp \
    simulation/atmosphere.cpp \
    simulation/hosekWilkie/ArHosekSkyModel.c \
    simulation/camera.cpp \
    simulation/crystalPopulation.cpp \
    simulation/crystalPopulationRepository.cpp \
    simulation/lightSource.cpp \
    simulation/simulationEngine.cpp \
    simulation/skyModel.cpp

RESOURCES = \
    resources/haloray.qrc
