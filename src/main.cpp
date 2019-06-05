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

const nk_flags WINDOW_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE;
const nk_flags GROUP_FLAGS = NK_WINDOW_BORDER | NK_WINDOW_TITLE;

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

void renderCrystalSettingsWindow(struct nk_context *ctx,
                                 HaloSim::CrystalPopulation &population)
{
    if (nk_begin(ctx, "Crystal settings", nk_rect(50, 650, 500, 370), WINDOW_FLAGS))
    {
        nk_layout_row_dynamic(ctx, 30, 2);
        nk_property_float(ctx, "#C/A ratio average:", 0.01f, &(population.caRatioAverage), 10.0f, 0.05f, 0.01f);
        nk_property_float(ctx, "#C/A ratio std:", 0.0f, &(population.caRatioStd), 10.0f, 0.05f, 0.01f);

        const char *distributions[] = {"Uniform", "Gaussian"};
        nk_layout_row_dynamic(ctx, 130, 1);
        if (nk_group_begin(ctx, "C axis orientation", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            population.polarAngleDistribution = nk_combo(ctx, distributions, NK_LEN(distributions), population.polarAngleDistribution, 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 90));
            if (population.polarAngleDistribution == 1)
            {
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_property_float(ctx, "#Average rotation:", 0.0f, &(population.polarAngleAverage), 360.0f, 0.1f, 0.5f);
                nk_property_float(ctx, "#Rotation std:", 0.0f, &(population.polarAngleStd), 360.0f, 0.1f, 0.5f);
            }
            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 130, 1);
        if (nk_group_begin(ctx, "Rotation around C axis", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            population.rotationDistribution = nk_combo(ctx, distributions, NK_LEN(distributions), population.rotationDistribution, 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 90));
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
                              HaloSim::Camera &camera)
{
    if (nk_begin(ctx, "View", nk_rect(50, 400, 330, 200), WINDOW_FLAGS))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Brightness", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.01f, &(exposure), 10.0f, 0.1f);

        nk_label(ctx, "Field of view", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.01f, &(camera.fov), 2.0f, 0.01f);
    }
    nk_end(ctx);
}

void renderGeneralSettingsWindow(struct nk_context *ctx,
                                 bool isRendering,
                                 int &numRays,
                                 int maxNumRays,
                                 HaloSim::LightSource &light,
                                 std::function<void()> renderButtonFn,
                                 std::function<void()> stopButtonFn)
{
    if (nk_begin(ctx, "General settings", nk_rect(50, 30, 330, 330), WINDOW_FLAGS))
    {
        nk_layout_row_dynamic(ctx, 120, 1);
        if (nk_group_begin(ctx, "Sun parameters", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_property_float(ctx, "#Altitude:", -90.0f, &(light.altitude), 90.0f, 0.05f, 0.1f);
            nk_property_float(ctx, "#Diameter:", 0.0f, &(light.diameter), 360.0f, 0.01f, 0.1f);

            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 100, 1);
        if (nk_group_begin(ctx, "Simulation parameters", GROUP_FLAGS))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_property_int(ctx, "#Number of rays:", 10000, &numRays, maxNumRays, 1000, 5000);

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
    }
    nk_end(ctx);
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
    const int maxNumRays = std::min(500000, maxComputeGroups);

    int numRays = std::min(400000, maxNumRays);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* Render simulation result texture */
        texDrawPrg->Use();
        glUniform1f(glGetUniformLocation(texDrawPrg->GetHandle(), "exposure"), exposure / iteration);
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
