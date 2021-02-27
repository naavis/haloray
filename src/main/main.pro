TARGET = HaloRay
TEMPLATE = app

QT += core gui widgets
CONFIG += c++17
win32:CONFIG += windows

DEFINES += QT_MESSAGELOGCONTEXT
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

SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../haloray-core/release/ -lHaloRayCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../haloray-core/debug/ -lHaloRayCore
else:unix: LIBS += -L$$OUT_PWD/../haloray-core/ -lHaloRayCore

INCLUDEPATH += $$PWD/../haloray-core
DEPENDPATH += $$PWD/../haloray-core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../haloray-core/release/libHaloRayCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../haloray-core/debug/libHaloRayCore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../haloray-core/release/HaloRayCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../haloray-core/debug/HaloRayCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../haloray-core/libHaloRayCore.a

RC_ICONS = ../haloray-core/resources/haloray.ico
