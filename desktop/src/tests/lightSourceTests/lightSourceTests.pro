TARGET = lightSourceTests
TEMPLATE = app
QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase c++17
win32:CONFIG += windows

SOURCES +=  \
    lightSourceTests.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../haloray-core/release/ -lHaloRayCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../haloray-core/debug/ -lHaloRayCore
else:unix: LIBS += -L$$OUT_PWD/../../haloray-core/ -lHaloRayCore

INCLUDEPATH += $$PWD/../../haloray-core
DEPENDPATH += $$PWD/../../haloray-core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../haloray-core/release/libHaloRayCore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../haloray-core/debug/libHaloRayCore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../haloray-core/release/HaloRayCore.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../haloray-core/debug/HaloRayCore.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../haloray-core/libHaloRayCore.a
