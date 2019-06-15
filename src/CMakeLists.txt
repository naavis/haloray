add_executable(haloray
    main.cpp
    gui.cpp
    simulation/simulationEngine.cpp
    opengl/utils.cpp
    opengl/texture.cpp
    opengl/shader.cpp
    opengl/program.cpp)
target_link_libraries(haloray ${CONAN_LIBS})
