# HaloRay

HaloRay simulates the reflection and refraction of sun light inside hexagonal ice crystals present
in high altitude clouds in the atmosphere. These ice crystals produce various optical phenomena
in the sky, including bright spots, circles and arcs.

HaloRay employs GPGPU to massively accelerate simulations. Brunt of the work happens in OpenGL compute shaders.

HaloRay currently supports Windows and Linux.

![Simulation of a column crystal halo display](images/column-halos.png)

## How to build?

HaloRay requires an OpenGL 4.4 compliant GPU.
The build is done using [CMake](https://cmake.org/).

# TODO: Add mention of Qt

Build the project by running:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
