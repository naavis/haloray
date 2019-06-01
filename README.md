# Halo Simulator Prototype

The simulator requires an OpenGL 4.4 compliant GPU.
Build dependencies are handled using [Conan](https://conan.io/) and the build itself is done with
[CMake](https://cmake.org/).

Conan must be configured with the Bincrafters remote:
```bash
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```


Build the project by running:

```bash
mkdir build
cd build
conan install --build=missing ..
cmake --build --config Release ..
```

