#pragma once
#include <functional>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear/nuklear.h"
#include "../simulation/simulationEngine.h"

void renderCrystalSettingsWindow(struct nk_context *ctx,
                                 HaloSim::CrystalPopulation &population);

void renderViewSettingsWindow(struct nk_context *ctx,
                              float &exposure,
                              bool &lockedToSun,
                              HaloSim::Camera &camera,
                              double framesPerSecond);

void renderGeneralSettingsWindow(struct nk_context *ctx,
                                 bool isRendering,
                                 int &numRays,
                                 int maxNumRays,
                                 HaloSim::LightSource &light,
                                 std::function<void()> renderButtonFn,
                                 std::function<void()> stopButtonFn);
