#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
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
#include "nuklear.h"
#include "nuklear_glfw.h"
#include "opengl/shader.h"
#include "opengl/program.h"

const std::string computeShaderSource =
#include "shaders/raytrace.glsl"
    ;

typedef struct
{
    float caRatioAverage;
    float caRatioStd;

    int polarAngleDistribution;
    float polarAngleAverage;
    float polarAngleStd;

    int rotationDistribution;
    float rotationAverage;
    float rotationStd;
} crystalProperties_t;

typedef struct
{
    float altitude;
    float azimuth;
    float diameter;
} sunProperties_t;

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

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

    OpenGL::Shader fragmentShader(fragShaderSrc, OpenGL::ShaderType::Fragment);
    fragmentShader.Compile();

    auto program = std::make_shared<OpenGL::Program>();
    program->AttachShader(vertexShader);
    program->AttachShader(fragmentShader);
    program->Link();

    return program;
}

std::shared_ptr<OpenGL::Program> createComputeShaderProgram()
{
    OpenGL::Shader shader(computeShaderSource, OpenGL::ShaderType::Compute);
    shader.Compile();

    auto program = std::make_shared<OpenGL::Program>();
    program->AttachShader(shader);
    program->Link();

    return program;
}

void runSimulation(std::shared_ptr<OpenGL::Program> computeShaderPrg, GLuint tex, GLuint spinlock, unsigned int numRays, sunProperties_t sun, crystalProperties_t crystalProperties)
{
    glClearTexImage(tex, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(spinlock, 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(1, spinlock, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    computeShaderPrg->Use();

    GLint maxNumGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxNumGroups);
    unsigned int finalNumGroups = (int)numRays > maxNumGroups ? maxNumGroups : numRays;
    GLuint shaderHandle = computeShaderPrg->GetProgramHandle();

    glUniform1f(glGetUniformLocation(shaderHandle, "sun.altitude"), sun.altitude);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.azimuth"), sun.azimuth);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.diameter"), sun.diameter);

    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioAverage"), crystalProperties.caRatioAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioStd"), crystalProperties.caRatioStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleDistribution"), crystalProperties.polarAngleDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleAverage"), crystalProperties.polarAngleAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleStd"), crystalProperties.polarAngleStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.rotationDistribution"), crystalProperties.rotationDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationAverage"), crystalProperties.rotationAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationStd"), crystalProperties.rotationStd);

    glDispatchCompute(finalNumGroups, 1, 1);
}

struct nk_context *initNuklear(GLFWwindow *window)
{
    struct nk_context *ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    return ctx;
}

void main(int argc, char const *argv[])
{
    /* Initialize GLFW */
    if (!glfwInit())
        exit(EXIT_FAILURE);

    GLFWwindow *window = createWindow();

    /* Initialize GLAD */
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    struct nk_context *ctx = initNuklear(window);

    GLuint simulationTexture;
    glGenTextures(1, &simulationTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, simulationTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint spinlockTexture;
    glGenTextures(1, &spinlockTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, spinlockTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1920, 1080, 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
    std::shared_ptr<OpenGL::Program> computeShaderPrg;

    try
    {
        texDrawPrg = createTexDrawShaderProgram();
        computeShaderPrg = createComputeShaderProgram();
    }
    catch (const std::runtime_error &e)
    {
        std::printf("Error occurred when creating shaders:\n%s\n", e.what());
        exit(EXIT_FAILURE);
    }

    const nk_flags windowFlags = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE;
    const nk_flags groupFlags = NK_WINDOW_BORDER | NK_WINDOW_TITLE;

    int numRays = 10000000;
    sunProperties_t sun;
    sun.altitude = 30.0f;
    sun.azimuth = 0.0f;
    sun.diameter = 0.5f;

    crystalProperties_t crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.polarAngleDistribution = 1;
    crystalProperties.polarAngleAverage = 0.0f;
    crystalProperties.polarAngleStd = 40.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 60.0f;
    crystalProperties.rotationStd = 1.0f;

    float exposure = 1.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* Render offscreen texture contents to default framebuffer */
        texDrawPrg->Use();
        glUniform1f(glGetUniformLocation(texDrawPrg->GetProgramHandle(), "exposure"), exposure);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVao);
        glBindTexture(GL_TEXTURE_2D, simulationTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Start rendering GUI */
        nk_glfw3_new_frame();

        if (nk_begin(ctx, "Crystal settings", nk_rect(400, 50, 500, 400), windowFlags))
        {
            nk_layout_row_dynamic(ctx, 30, 2);
            nk_property_float(ctx, "#C/A ratio average:", 0.01f, &(crystalProperties.caRatioAverage), 10.0f, 0.05f, 0.01f);
            nk_property_float(ctx, "#C/A ratio std:", 0.0f, &(crystalProperties.caRatioStd), 10.0f, 0.05f, 0.01f);

            const char *distributions[] = {"Uniform", "Gaussian"};
            nk_layout_row_dynamic(ctx, 130, 1);
            if (nk_group_begin(ctx, "C axis orientation", groupFlags))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                crystalProperties.polarAngleDistribution = nk_combo(ctx, distributions, NK_LEN(distributions), crystalProperties.polarAngleDistribution, 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 90));
                if (crystalProperties.polarAngleDistribution == 1)
                {
                    nk_layout_row_dynamic(ctx, 30, 2);
                    nk_property_float(ctx, "#Average rotation:", 0.0f, &(crystalProperties.polarAngleAverage), 360.0f, 0.1f, 0.5f);
                    nk_property_float(ctx, "#Average std:", 0.0f, &(crystalProperties.polarAngleStd), 360.0f, 0.1f, 0.5f);
                }
                nk_group_end(ctx);
            }

            nk_layout_row_dynamic(ctx, 130, 1);
            if (nk_group_begin(ctx, "Rotation around C axis", groupFlags))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                crystalProperties.rotationDistribution = nk_combo(ctx, distributions, NK_LEN(distributions), crystalProperties.rotationDistribution, 30, nk_vec2(nk_layout_widget_bounds(ctx).w, 90));
                if (crystalProperties.rotationDistribution == 1)
                {
                    nk_layout_row_dynamic(ctx, 30, 2);
                    nk_property_float(ctx, "#Average rotation:", 0.0f, &(crystalProperties.rotationAverage), 360.0f, 0.1f, 0.5f);
                    nk_property_float(ctx, "#Average std:", 0.0f, &(crystalProperties.rotationStd), 360.0f, 0.1f, 0.5f);
                }
                nk_group_end(ctx);
            }
        }
        nk_end(ctx);

        if (nk_begin(ctx, "General settings", nk_rect(50, 50, 330, 430), windowFlags))
        {
            nk_layout_row_dynamic(ctx, 150, 1);
            if (nk_group_begin(ctx, "Sun parameters", groupFlags))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_property_float(ctx, "#Altitude:", -90.0f, &(sun.altitude), 90.0f, 0.05f, 0.1f);
                nk_property_float(ctx, "#Azimuth:", 0.0f, &(sun.azimuth), 360.0f, 0.05f, 0.1f);
                nk_property_float(ctx, "#Diameter:", 0.0f, &(sun.diameter), 360.0f, 0.01f, 0.1f);

                nk_group_end(ctx);
            }

            nk_layout_row_dynamic(ctx, 100, 1);
            if (nk_group_begin(ctx, "Simulation parameters", groupFlags))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_property_int(ctx, "#Number of rays:", 10000, &numRays, 40000000, 10000, 50000);

                nk_group_end(ctx);
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Brightness", NK_TEXT_LEFT);
            nk_slider_float(ctx, 0.01f, &exposure, 10.0f, 0.1f);
            if (nk_button_label(ctx, "Render"))
            {
                runSimulation(computeShaderPrg, simulationTexture, spinlockTexture, numRays, sun, crystalProperties);
            }
        }
        nk_end(ctx);

        nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &simulationTexture);
    glDeleteTextures(1, &spinlockTexture);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quadVao);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &quadVbo);

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
