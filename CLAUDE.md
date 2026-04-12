# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HaloRay is a C++17 desktop application that simulates atmospheric ice halos (sun halos) using GPU-accelerated ray tracing. It uses Qt 5 for the GUI and OpenGL 4.4 compute shaders for the simulation.

## Build Commands

The build system uses qmake (Qt). On Windows with MSVC:

```bash
# Build
mkdir build
cd src
qmake haloray.pro -o ../build/ -config release
cd ../build
nmake

# Run tests
nmake check

# CI uses PowerShell scripts in scripts/ (build.ps1, test.ps1, package.ps1)
```

Requires Qt 5.15 and a GPU with OpenGL 4.4 support. CI runs on AppVeyor with MSVC 2019.

## Architecture

The project is split into three qmake subprojects (`src/haloray.pro`):
- **main** — Application entry point, depends on haloray-core
- **haloray-core** — Static library containing all application logic
- **tests** — Qt Test suite, depends on haloray-core

### Key modules inside haloray-core (`src/haloray-core/`):

- **simulation/** — Core domain: `SimulationEngine` orchestrates GPU ray tracing. Key types: `Camera` (5 projection modes), `CrystalPopulation` (ice crystal parameters with 6 presets), `LightSource`, `Atmosphere` (Hosek-Wilkie sky model).
- **gui/** — Qt widgets: `MainWindow`, settings panels, `OpenGLWidget` (rendering surface), `SimulationStateModel` (bridges UI and simulation state via Qt signals/slots), `StateSaver` (persists parameters to disk).
- **opengl/** — GPU abstractions: `Texture`, `TextureRenderer`.
- **resources/shaders/** — GLSL 440 compute shaders: `raytrace.glsl` (ray tracing kernel, 64 threads/group), `sky.glsl`, `guide.glsl`, and display shaders.

### Data flow

User adjusts parameters in settings widgets → `SimulationStateModel` updates → `SimulationEngine` reconfigures GPU state → compute shaders run ray tracing → results accumulate in textures → `OpenGLWidget` displays output.

## Testing

Tests use Qt Test (`QTest`). Three test suites under `src/tests/`: `cameraTests`, `crystalPopulationRepositoryTests`, `lightSourceTests`. Each is a separate executable built via its own `.pro` file.
