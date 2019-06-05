add_executable(halosim
    main.cpp
    gui.cpp
    simulation/simulationEngine.cpp
    opengl/utils.cpp
    opengl/texture.cpp
    opengl/shader.cpp
    opengl/program.cpp)
target_link_libraries(halosim ${CONAN_LIBS})
