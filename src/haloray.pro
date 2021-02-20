TEMPLATE = subdirs
SUBDIRS += \
    main \
    haloray-core \
    tests

main.depends = haloray-core
tests.depends = haloray-core
