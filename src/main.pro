TEMPLATE = subdirs
SUBDIRS += \
    haloray-qml \
    haloray \
    haloray-core \
    tests

haloray.depends = haloray-core
tests.depends = haloray-core
