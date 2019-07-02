#include "textureRenderer.h"
#include <memory>
#include <string>
#include <QOpenGLShaderProgram>

namespace OpenGL
{

std::unique_ptr<QOpenGLShaderProgram> TextureRenderer::InitializeTexDrawShaderProgram()
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
        "    vec3 xyz = texelFetch(s, ivec2(gl_FragCoord.xy), 0).xyz;"
        "    mat3 xyzToSrgb = mat3(3.2406, -0.9689, 0.0557, -1.5372, 1.8758, -0.2040, -0.4986, 0.0415, 1.0570);"
        "    vec3 linearSrgb = xyzToSrgb * xyz * exposure;"
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

void TextureRenderer::Initialize()
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

    glGenVertexArrays(1, &mQuadVao);
    glBindVertexArray(mQuadVao);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &mQuadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadVbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    mTexDrawProgram = InitializeTexDrawShaderProgram();
}

void TextureRenderer::Render(unsigned int textureHandle)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Render simulation result texture */

    mTexDrawProgram->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mQuadVao);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void TextureRenderer::SetUniformFloat(std::string name, float value)
{
    mTexDrawProgram->bind();
    mTexDrawProgram->setUniformValue(name.c_str(), value);
}

TextureRenderer::~TextureRenderer()
{
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mQuadVao);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &mQuadVbo);
}

} // namespace OpenGL
