set (HaloRay_VERSION_MAJOR 1)
set (HaloRay_VERSION_MINOR 1)
set (HaloRay_VERSION_PATCH 0)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

add_executable(haloray
    main.cpp
    gui/gui.cpp
    simulation/simulationEngine.cpp
    opengl/utils.cpp
    opengl/texture.cpp
    opengl/shader.cpp
    opengl/textureRenderer.cpp
    opengl/program.cpp)
target_link_libraries(haloray ${CONAN_LIBS})
