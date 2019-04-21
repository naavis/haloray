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
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl4.h"

const std::string computeShaderSource =
#include "shaders/raytrace.glsl"
;

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

GLFWwindow* createWindow() {
    GLFWwindow* window;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

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

void checkCompileError(GLuint shader) {
    int rvalue;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling shader\n");
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(shader, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);
        exit(40);
    }
}

void checkLinkError(GLuint program) {
    int rvalue;
    glGetProgramiv(program, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking shader program\n");
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(program, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);
        exit(41);
    }
}

GLuint createTexDrawShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    static const GLchar* vertexShaderSrc =
        "#version 460 core\n"
        "in vec2 position;"
        "void main(void) {"
        "   gl_Position = vec4(position, 0.0f, 1.0);"
        "}";
    glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
    glCompileShader(vertexShader);
    checkCompileError(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    static const GLchar* fragShaderSrc =
        "#version 460 core\n"
        "out vec4 color;"
        "uniform sampler2D s;"
        "void main(void) {"
        "   color = texelFetch(s, ivec2(gl_FragCoord.xy), 0);"
        "}";
    glShaderSource(fragmentShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragmentShader);
    checkCompileError(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    checkLinkError(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint createComputeShader() {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    static const GLchar* src = computeShaderSource.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    checkCompileError(shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    checkLinkError(program);

    glDeleteShader(shader);

    return program;
}

void renderOffscreen(GLuint framebuffer, GLuint computeShader, GLuint tex) {
        /* Bind to off screen framebuffer */
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        /* Render to offscreen buffer here */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(computeShader);
        glDispatchCompute(200, 200, 1);
}

void renderGUI(struct nk_context* ctx) {
    nk_glfw3_new_frame();

    if (nk_begin(ctx, "Parameters", nk_rect(50, 50, 230, 250), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_static(ctx, 30, 200, 1);
        nk_button_label(ctx, "This is a button");
    }
    nk_end(ctx);

    nk_glfw3_render(NK_ANTI_ALIASING_ON);
}

void main(int argc, char const *argv[])
{
    /* Initialize GLFW */
    if (!glfwInit())
        exit(EXIT_FAILURE);

    GLFWwindow* window = createWindow();

    /* Initialize GLAD */
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    /* Initialize Nuklear */
    struct nk_context* ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&atlas);
        nk_glfw3_font_stash_end();
    }

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint framebufferColorTexture;
    glGenTextures(1, &framebufferColorTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferColorTexture, 0);

    static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, draw_buffers);

    float points[] = {
        -1.0f,  -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
    };

    GLuint texDrawPrg = createTexDrawShaderProgram();
    GLuint quadVao;
    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);
    glEnableVertexAttribArray(0);

    GLuint quadVbo;
    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint computeShader = createComputeShader();

    renderOffscreen(framebuffer, computeShader, framebufferColorTexture);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Bind to default framebuffer */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* Render offscreen buffer contents to default framebuffer */
        glUseProgram(texDrawPrg);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(quadVao);
        glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        renderGUI(ctx);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glUseProgram(0);
    glDeleteProgram(texDrawPrg);
    glDeleteProgram(computeShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &framebufferColorTexture);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quadVao);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &quadVbo);

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
