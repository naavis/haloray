#include "textureRenderer.h"
#include <memory>
#include <string>
#include <stdexcept>

namespace OpenGL
{

std::unique_ptr<QOpenGLShaderProgram> TextureRenderer::initializeTexDrawShaderProgram()
{
    const std::string vertexShaderSrc =
        "#version 440 core\n"
        "in vec2 position;"
        "void main(void) {"
        "    gl_Position = vec4(position, 0.0f, 1.0);"
        "}";

    const std::string fragShaderSrc =
        "#version 440 core\n"
        "out vec4 color;"
        "uniform float exposure;"
        "uniform sampler2D s;"
        "void main(void) {"
        "    vec3 linearSrgb = exposure * texelFetch(s, ivec2(gl_FragCoord.xy), 0).xyz;"
        "    vec3 gammaCorrected = pow(linearSrgb, vec3(0.42));"
        "    color = vec4(gammaCorrected, 1.0);"
        "}";

    auto program = std::make_unique<QOpenGLShaderProgram>();
    program->addCacheableShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Vertex, vertexShaderSrc.c_str());
    program->addCacheableShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Fragment, fragShaderSrc.c_str());

    if (program->link() == false)
    {
        throw std::runtime_error(program->log().toUtf8());
    }

    return program;
}

TextureRenderer::TextureRenderer()
{
    initialize();
}

void TextureRenderer::initialize()
{
    initializeOpenGLFunctions();
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

    glGenVertexArrays(1, &m_quadVao);
    glBindVertexArray(m_quadVao);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    m_texDrawProgram = initializeTexDrawShaderProgram();
}

void TextureRenderer::render(unsigned int textureHandle)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Render simulation result texture */

    m_texDrawProgram->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(m_quadVao);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void TextureRenderer::setUniformFloat(std::string name, float value)
{
    m_texDrawProgram->bind();
    m_texDrawProgram->setUniformValue(name.c_str(), value);
}

TextureRenderer::~TextureRenderer()
{
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_quadVao);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_quadVbo);
}

}
