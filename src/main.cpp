#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "opengl/textureRenderer.h"
#include "simulation/simulationEngine.h"
#include "gui/gui.h"
#include "version.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "gui/nuklear/nuklear.h"
#include "gui/nuklear/nuklear_glfw.h"

#include <QApplication>
#include <QMainWindow>
#include "gui/ui_mainwindow.h"

struct CallbackState
{
    HaloSim::SimulationEngine *engine;
    bool *showInterface;
};

void windowResizeCallback(GLFWwindow *window, int width, int height)
{
    auto state = reinterpret_cast<struct CallbackState *>(glfwGetWindowUserPointer(window));
    state->engine->ResizeOutputTextureCallback(width, height);
}

void keyboardCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
    auto state = reinterpret_cast<struct CallbackState *>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        *(state->showInterface) = !*(state->showInterface);
    }
}

GLFWwindow *createWindow()
{
    GLFWwindow *window;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    /* Create a windowed mode window and its OpenGL context */

    char titleBuffer[100];
    std::sprintf(
        titleBuffer,
        "HaloRay %i.%i.%i",
        HaloRay_VERSION_MAJOR,
        HaloRay_VERSION_MINOR,
        HaloRay_VERSION_PATCH);

    window = glfwCreateWindow(1920, 1080, titleBuffer, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    return window;
}

struct nk_context *initNuklear(GLFWwindow *window)
{
    struct nk_context *ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    return ctx;
}

void runMainLoop(GLFWwindow *window,
                 struct nk_context *ctx)
{
    int initialWidth, initialHeight;
    glfwGetWindowSize(window, &initialWidth, &initialHeight);
    HaloSim::SimulationEngine engine(initialWidth, initialHeight);

    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyboardCallback);

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

    int maxComputeGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeGroups);
    const int absoluteMaxNumRays = 1000000;
    const int maxNumRays = std::min(absoluteMaxNumRays, maxComputeGroups);

    const int defaultNumRays = 500000;
    int numRays = std::min(defaultNumRays, maxNumRays);

    CallbackState cbState;
    cbState.engine = &engine;
    cbState.showInterface = &showInterface;

    glfwSetWindowUserPointer(window, &cbState);

    double framesPerSecond = 0.0;
    auto currentTime = glfwGetTime();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        texRenderer.SetUniformFloat("exposure", exposure / (iteration + 1) / camera.fov);
        texRenderer.Render(engine.GetOutputTextureHandle());

        /* Start rendering GUI */
        nk_glfw3_new_frame();

        if (lockedToSun)
        {
            camera.pitch = sun.altitude;
            camera.yaw = 0.0f;
        }

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

        if (showInterface)
        {
            GUI::renderCrystalSettingsWindow(ctx, crystalProperties);
            GUI::renderViewSettingsWindow(ctx, exposure, lockedToSun, camera, framesPerSecond);
        }

        auto renderButtonFn = [&engine, &iteration, &isRendering]() {
            engine.Clear();
            iteration = 0;
            isRendering = true;
        };

        auto stopButtonFn = [&isRendering]() {
            isRendering = false;
        };

        if (showInterface)
        {
            GUI::renderGeneralSettingsWindow(ctx, isRendering, numRays, maxNumRays, maxNumIterations, iteration, sun, renderButtonFn, stopButtonFn);
        }

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

        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        /* Update frames per second counter */
        auto newTime = glfwGetTime();
        framesPerSecond = 1.0 / (newTime - currentTime);
        currentTime = newTime;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow mainWindow;
    Ui::MainWindow ui;
    ui.setupUi(&mainWindow);

    mainWindow.show();

    return app.exec();

    /* Initialize GLFW */
    if (!glfwInit())
        exit(EXIT_FAILURE);

    GLFWwindow *window = createWindow();

    /* Initialize GLAD */
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    /* Initialize Nuklear context */
    struct nk_context *ctx = initNuklear(window);

    runMainLoop(window, ctx);

    nk_glfw3_shutdown();
    glfwTerminate();
    return 0;
}
