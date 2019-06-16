#include "gui.h"
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

namespace GUI {

const nk_flags WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE;
const nk_flags GROUP_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_TITLE;

void renderGeneralSettingsWindow(struct nk_context *ctx,
                                 bool isRendering,
                                 int &numRays,
                                 const int maxNumRays,
                                 int &maxNumIterations,
                                 const int currentIteration,
                                 HaloSim::LightSource &light,
                                 std::function<void()> renderButtonFn,
                                 std::function<void()> stopButtonFn)
{
    if (nk_begin(ctx, "General settings", nk_rect(50, 20, 330, 430), WINDOW_FLAGS))
    {
        nk_layout_row_dynamic(ctx, 120, 1);
        if (nk_group_begin(ctx, "Sun parameters", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_property_float(ctx, "#Altitude:", -90.0f, &(light.altitude), 90.0f, 0.05f, 0.1f);
            nk_property_float(ctx, "#Diameter:", 0.0f, &(light.diameter), 360.0f, 0.01f, 0.1f);

            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 120, 1);
        if (nk_group_begin(ctx, "Simulation parameters", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_property_int(ctx, "#Rays per frame:", 10000, &numRays, maxNumRays, 1000, 5000);
            nk_property_int(ctx, "#Maximum number of frames:", 1, &maxNumIterations, 1000000, 1, 10);

            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 50, 1);
        if (isRendering)
        {
            if (nk_button_label(ctx, "Stop"))
            {
                stopButtonFn();
            }
        }
        else
        {
            if (nk_button_label(ctx, "Render"))
            {
                renderButtonFn();
            }
        }
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Progress", NK_TEXT_LEFT);
        nk_progress(ctx, (nk_size *)&currentIteration, maxNumIterations, nk_false);
    }
    nk_end(ctx);
}

void renderCrystalSettingsWindow(struct nk_context *ctx,
                                 HaloSim::CrystalPopulation &population)
{
    if (nk_begin(ctx, "Crystal settings", nk_rect(50, 710, 500, 330), WINDOW_FLAGS))
    {
        nk_layout_row_dynamic(ctx, 30, 2);
        nk_property_float(ctx, "#C/A ratio average:", 0.01f, &(population.caRatioAverage), 10.0f, 0.05f, 0.01f);
        nk_property_float(ctx, "#C/A ratio std:", 0.0f, &(population.caRatioStd), 10.0f, 0.05f, 0.01f);

        const char *distributions[] = {"Uniform", "Gaussian"};
        nk_layout_row_dynamic(ctx, 120, 1);
        if (nk_group_begin(ctx, "C axis orientation", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_combobox(ctx, distributions, NK_LEN(distributions), &(population.polarAngleDistribution), 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 90));
            if (population.polarAngleDistribution == 1)
            {
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_property_float(ctx, "#Average rotation:", 0.0f, &(population.polarAngleAverage), 360.0f, 0.1f, 0.5f);
                nk_property_float(ctx, "#Rotation std:", 0.0f, &(population.polarAngleStd), 360.0f, 0.1f, 0.5f);
            }
            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 120, 1);
        if (nk_group_begin(ctx, "Rotation around C axis", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_combobox(ctx, distributions, NK_LEN(distributions), &(population.rotationDistribution), 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 90));
            if (population.rotationDistribution == 1)
            {
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_property_float(ctx, "#Average rotation:", 0.0f, &(population.rotationAverage), 360.0f, 0.1f, 0.5f);
                nk_property_float(ctx, "#Rotation std:", 0.0f, &(population.rotationStd), 360.0f, 0.1f, 0.5f);
            }
            nk_group_end(ctx);
        }
    }
    nk_end(ctx);
}

void renderViewSettingsWindow(struct nk_context *ctx,
                              float &exposure,
                              bool &lockedToSun,
                              HaloSim::Camera &camera,
                              double framesPerSecond)
{
    if (nk_begin(ctx, "View", nk_rect(1550, 700, 330, 320), WINDOW_FLAGS))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Brightness", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.01f, &(exposure), 10.0f, 0.1f);

        nk_label(ctx, "Field of view", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.01f, &(camera.fov), 2.0f, 0.01f);

        int locked = lockedToSun ? nk_true : nk_false;
        nk_checkbox_label(ctx, "Lock view to sun", &locked);
        lockedToSun = locked == nk_true;

        int hideSubHorizon = camera.hideSubHorizon ? nk_true : nk_false;
        nk_checkbox_label(ctx, "Hide sub-horizon", &hideSubHorizon);
        camera.hideSubHorizon = hideSubHorizon == nk_true;

        const char *projections[] = {
            "Stereographic",
            "Rectilinear",
            "Equidistant",
            "Equal area",
            "Orthographic"};

        int projectionIndex = (int)camera.projection;
        nk_combobox(ctx, projections, NK_LEN(projections), &(projectionIndex), 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 300));
        camera.projection = (HaloSim::Projection)projectionIndex;

        nk_labelf(ctx, NK_TEXT_LEFT, "Frames per second: %.2f", framesPerSecond);
    }
    nk_end(ctx);
}

}
