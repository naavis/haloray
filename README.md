# HaloRay

HaloRay simulates the reflection and refraction of sun light inside hexagonal ice crystals present
in high altitude clouds in the atmosphere. These ice crystal produce various optical phenomena
in the sky, including bright spots, circles and arcs.

HaloRay employs GPGPU to massively accelerate simulations. Brunt of the work happens in OpenGL compute shaders.

## How to build?

HaloRay requires an OpenGL 4.4 compliant GPU.
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
cmake ..
cmake --build . --config Release
```

