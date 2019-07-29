add_executable(cameraTests cameraTests.cpp)
target_link_libraries(cameraTests Qt5::Core Qt5::Test halorayLib)
add_test(cameraTests cameraTests)

add_executable(crystalRepoTests crystalPopulationRepositoryTests.cpp)
target_link_libraries(crystalRepoTests Qt5::Core Qt5::Test halorayLib)
add_test(crystalRepoTests crystalRepoTests)
