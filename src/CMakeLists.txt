add_executable(halosim
    main.cpp
    opengl/utils.cpp
    opengl/shader.cpp
    opengl/program.cpp)
target_link_libraries(halosim ${CONAN_LIBS})
