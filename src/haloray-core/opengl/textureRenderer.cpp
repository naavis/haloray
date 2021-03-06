#include "textureRenderer.h"
#include <memory>
#include <string>
#include <QtGlobal>
#include <stdexcept>

namespace OpenGL
{

std::unique_ptr<QOpenGLShaderProgram> TextureRenderer::initializeTexDrawShaderProgram()
{
    qInfo("Initializing texture renderer vertex shader");
    auto program = std::make_unique<QOpenGLShaderProgram>();
    bool vertexShaderReadSucceeded = program->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Vertex, ":/shaders/renderer.vert");

    if (vertexShaderReadSucceeded == false)
    {
        qWarning("Texture renderer vertex shader read failed");
        throw std::runtime_error(program->log().toUtf8());
    }
    qInfo("Texture renderer vertex shader successfully initialized");

    qInfo("Initializing texture renderer fragment shader");
    bool fragmentShaderReadSucceeded = program->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Fragment, ":/shaders/renderer.frag");
    if (fragmentShaderReadSucceeded == false)
    {
        qWarning("Texture renderer fragment shader read failed");
        throw std::runtime_error(program->log().toUtf8());
    }
    qInfo("Texture renderer fragment shader successfully initialized");

    if (program->link() == false)
    {
        qWarning("Texture renderer shader compilation and linking failed");
        throw std::runtime_error(program->log().toUtf8());
    }
    qInfo("Texture renderer shader program compilation and linking successful");

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

void TextureRenderer::render(unsigned int haloTextureHandle, int backgroundTextureHandle)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Render simulation result texture */

    m_texDrawProgram->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(m_quadVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, haloTextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glActiveTexture(GL_TEXTURE0);
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
