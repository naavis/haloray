TEMPLATE = subdirs
SUBDIRS += \
    haloray \
    haloray-core \
    tests

haloray.depends = haloray-core
tests.depends = haloray-core
