set(TEST_SOURCES
    cameraTests.cpp
)

add_executable(halorayTests ${TEST_SOURCES})
target_link_libraries(halorayTests Qt5::Core Qt5::Test halorayLib)
add_test(halorayTests halorayTests)
