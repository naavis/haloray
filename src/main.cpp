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

GLuint createExampleShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    static const GLchar* vertexShaderSrc =
        "#version 460 core\n"
        "void main(void) {\n"
        "   gl_Position = vec4(0.0, 0.0, 0.5, 1.0);\n"
        "}\n";
    glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    static const GLchar* fragShaderSrc =
        "#version 460 core\n"
        "out vec4 color;\n"
        "void main(void) {\n"
        "   color = vec4(0.0, 0.8, 1.0, 1.0);\n"
        "}\n";
    glShaderSource(fragmentShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
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
    struct nk_context *ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&atlas);
        nk_glfw3_font_stash_end();
    }

    GLuint examplePrg = createExampleShaderProgram();

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint framebufferColorTexture;
    glGenTextures(1, &framebufferColorTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1920, 1080);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebufferColorTexture, 0);

    static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, draw_buffers);

    GLuint exampleVbo;
    glGenVertexArrays(1, &exampleVbo);
    glBindVertexArray(exampleVbo);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(0);
        /* Bind to off screen framebuffer */
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT);

        /* TODO: Render to offscreen buffer here */

        /* TODO: Render offscreen buffer contents to default framebuffer */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(examplePrg);
        glBindVertexArray(exampleVbo);
        glPointSize(40.0f);
        glDrawArrays(GL_POINTS, 0, 1);

        /* Start defining UI */
        nk_glfw3_new_frame();

        if (nk_begin(ctx, "Parameters", nk_rect(50, 50, 230, 250), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            nk_layout_row_static(ctx, 30, 200, 1);
            nk_button_label(ctx, "This is a button");
        }
        nk_end(ctx);

        /* Render UI here */
        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(examplePrg);

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
