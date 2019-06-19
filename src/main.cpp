#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include "opengl/textureRenderer.h"
#include "simulation/simulationEngine.h"
#include "version.h"
#include "gui/mainWindow.h"

void runMainLoop()
{
    HaloSim::SimulationEngine engine(1920, 1080);

    try
    {
        engine.Initialize();
    }
    catch (const std::runtime_error &e)
    {
        std::printf("Could not initialize simulation engine: %s", e.what());
        exit(EXIT_FAILURE);
    }

    OpenGL::TextureRenderer texRenderer;
    try
    {
        texRenderer.Initialize();
    }
    catch (const std::runtime_error &e)
    {
        std::printf("Could not initialize texture renderer: %s", e.what());
        exit(EXIT_FAILURE);
    }

    HaloSim::Camera camera;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.fov = 0.5f;
    camera.projection = HaloSim::Projection::Stereographic;
    camera.hideSubHorizon = false;

    HaloSim::LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;

    HaloSim::CrystalPopulation crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.tiltDistribution = 1;
    crystalProperties.tiltAverage = 0.0f;
    crystalProperties.tiltStd = 40.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 0.0f;
    crystalProperties.rotationStd = 1.0f;

    float exposure = 1.0f;
    int iteration = 0;
    int maxNumIterations = 1000;
    bool isRendering = false;
    bool lockedToSun = false;
    bool showInterface = true;

    /*
    int maxComputeGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeGroups);
    const int absoluteMaxNumRays = 1000000;
    const int maxNumRays = std::min(absoluteMaxNumRays, maxComputeGroups);

    const int defaultNumRays = 500000;
    int numRays = std::min(defaultNumRays, maxNumRays);
*/
    int numRays = 500000;

    /* Loop until the user closes the window */
    while (true)
    {
        texRenderer.SetUniformFloat("exposure", exposure / (iteration + 1) / camera.fov);
        texRenderer.Render(engine.GetOutputTextureHandle());

        if (lockedToSun)
        {
            camera.pitch = sun.altitude;
            camera.yaw = 0.0f;
        }

        /*
        if (isRendering && nk_window_is_any_hovered(ctx) == nk_false)
        {
            const float zoomSpeed = 0.1f * camera.fov;
            camera.fov -= zoomSpeed * ctx->input.mouse.scroll_delta.y;
            camera.fov = std::max(camera.fov, 0.01f);
            camera.fov = std::min(camera.fov, 2.0f);
        }

        if (isRendering)
        {
            static bool justStartedDraggingMouse = true;
            static bool dragStartedOnBackground = true;
            if (ctx->input.mouse.buttons[0].down == nk_true && !lockedToSun)
            {
                if (justStartedDraggingMouse)
                {
                    dragStartedOnBackground = nk_window_is_any_hovered(ctx) == nk_false;
                    justStartedDraggingMouse = false;
                }

                if (dragStartedOnBackground)
                {
                    float xDelta = ctx->input.mouse.delta.x;
                    float yDelta = ctx->input.mouse.delta.y;
                    camera.yaw += 0.2f * xDelta * camera.fov;
                    camera.pitch += 0.2f * yDelta * camera.fov;
                }
            }
            else
            {
                justStartedDraggingMouse = true;
                dragStartedOnBackground = true;
            }
        }
*/

        auto renderButtonFn = [&engine, &iteration, &isRendering]() {
            engine.Clear();
            iteration = 0;
            isRendering = true;
        };

        auto stopButtonFn = [&isRendering]() {
            isRendering = false;
        };

        if (isRendering)
        {
            if (crystalProperties != engine.GetCrystalPopulation())
            {
                engine.SetCrystalPopulation(crystalProperties);
                iteration = 0;
            }

            if (sun != engine.GetLightSource())
            {
                engine.SetLightSource(sun);
                iteration = 0;
            }

            if (camera != engine.GetCamera())
            {
                engine.SetCamera(camera);
                iteration = 0;
            }

            if (iteration < maxNumIterations)
            {
                engine.Run(numRays);
                ++iteration;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
