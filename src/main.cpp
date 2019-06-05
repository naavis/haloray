#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "opengl/shader.h"
#include "opengl/program.h"
#include "simulation/simulationEngine.h"
#include "gui.h"

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
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_glfw.h"

GLFWwindow *createWindow()
{
    GLFWwindow *window;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Halo Sim Prototype", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    return window;
}

std::shared_ptr<OpenGL::Program> createTexDrawShaderProgram()
{
    const std::string vertexShaderSrc =
        "#version 440 core\n"
        "in vec2 position;"
        "void main(void) {"
        "    gl_Position = vec4(position, 0.0f, 1.0);"
        "}";

    OpenGL::Shader vertexShader(vertexShaderSrc, OpenGL::ShaderType::Vertex);
    vertexShader.Compile();

    const std::string fragShaderSrc =
        "#version 440 core\n"
        "out vec4 color;"
        "uniform float exposure;"
        "uniform sampler2D s;"
        "void main(void) {"
        "    vec3 xyz = texelFetch(s, ivec2(gl_FragCoord.xy), 0).xyz;"
        "    mat3 xyzToSrgb = mat3(3.2406, -0.9689, 0.0557, -1.5372, 1.8758, -0.2040, -0.4986, 0.0415, 1.0570);"
        "    vec3 linearSrgb = xyzToSrgb * xyz * exposure;"
        "    vec3 gammaCorrected = pow(linearSrgb, vec3(0.42));"
        "    color = vec4(gammaCorrected, 1.0);"
        "}";

    OpenGL::Shader fragmentShader(fragShaderSrc,
                                  OpenGL::ShaderType::Fragment);
    fragmentShader.Compile();

    auto program = std::make_shared<OpenGL::Program>();
    program->AttachShader(vertexShader);
    program->AttachShader(fragmentShader);
    program->Link();

    return program;
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
    HaloSim::SimulationEngine engine;
    try
    {
        engine.Initialize();
    }
    catch (const std::runtime_error &e)
    {
        std::printf("Could not initialize simulation engine: %s", e.what());
        exit(EXIT_FAILURE);
    }

    float points[] = {
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
    };

    GLuint quadVao;
    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);
    glEnableVertexAttribArray(0);

    GLuint quadVbo;
    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    std::shared_ptr<OpenGL::Program> texDrawPrg;

    try
    {
        texDrawPrg = createTexDrawShaderProgram();
    }
    catch (const std::runtime_error &e)
    {
        std::printf("Error occurred when creating texture drawing shader:\n%s\n", e.what());
        exit(EXIT_FAILURE);
    }

    HaloSim::Camera camera;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.fov = 0.5f;
    camera.projection = HaloSim::Projection::Stereographic;

    HaloSim::LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;

    HaloSim::CrystalPopulation crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.polarAngleDistribution = 1;
    crystalProperties.polarAngleAverage = 0.0f;
    crystalProperties.polarAngleStd = 40.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 60.0f;
    crystalProperties.rotationStd = 1.0f;

    float exposure = 1.0f;
    int iteration = 1;
    bool isRendering = false;

    int maxComputeGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeGroups);
    const int defaultMaxNumRays = 500000;
    const int maxNumRays = std::min(defaultMaxNumRays, maxComputeGroups);

    const int defaultNumRays = 400000;
    int numRays = std::min(defaultMaxNumRays, maxNumRays);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* Render simulation result texture */
        texDrawPrg->Use();
        glUniform1f(glGetUniformLocation(texDrawPrg->GetHandle(), "exposure"), exposure / iteration / camera.fov);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVao);
        glBindTexture(GL_TEXTURE_2D, engine.GetOutputTextureHandle());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Start rendering GUI */
        nk_glfw3_new_frame();

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
            if (ctx->input.mouse.buttons[0].down == nk_true)
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

        renderCrystalSettingsWindow(ctx, crystalProperties);
        renderViewSettingsWindow(ctx, exposure, camera);

        auto renderButtonFn = [&engine, &iteration, &isRendering]() {
            engine.Clear();
            iteration = 1;
            isRendering = true;
        };

        auto stopButtonFn = [&isRendering]() {
            isRendering = false;
        };

        renderGeneralSettingsWindow(ctx, isRendering, numRays, maxNumRays, sun, renderButtonFn, stopButtonFn);

        if (isRendering)
        {
            ++iteration;

            if (crystalProperties != engine.GetCrystalPopulation())
            {
                engine.SetCrystalPopulation(crystalProperties);
                iteration = 1;
            }

            if (sun != engine.GetLightSource())
            {
                engine.SetLightSource(sun);
                iteration = 1;
            }

            if (camera != engine.GetCamera())
            {
                engine.SetCamera(camera);
                iteration = 1;
            }

            engine.Run(numRays);
        }

        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quadVao);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &quadVbo);
}

int main(int argc, char const *argv[])
{
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
