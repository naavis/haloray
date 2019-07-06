set(TEST_SOURCES
    cameraTests.cpp
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests Qt5::Core Qt5::Test halorayLib)
add_test(tests tests)
