#include "simulationEngine.h"
#include <glad/glad.h>
#include "opengl/program.h"
#include "opengl/texture.h"

#include "shaders/raytrace.glsl"

namespace HaloSim
{

const unsigned int SimulationEngine::GetOutputTextureHandle() const
{
    return mSimulationTexture->GetHandle();
}

void SimulationEngine::Run(crystalProperties_t crystal, sunProperties_t sun, unsigned int numRays) const
{
    glClearTexImage(mSimulationTexture->GetHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(mSimulationTexture->GetTextureUnit(), mSimulationTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->GetHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->GetTextureUnit(), mSpinlockTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    mSimulationShader->Use();

    GLint maxNumGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxNumGroups);
    unsigned int finalNumGroups = (int)numRays > maxNumGroups ? maxNumGroups : numRays;
    GLuint shaderHandle = mSimulationShader->GetHandle();

    glUniform1f(glGetUniformLocation(shaderHandle, "sun.altitude"), sun.altitude);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.azimuth"), sun.azimuth);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.diameter"), sun.diameter);

    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioAverage"), crystal.caRatioAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioStd"), crystal.caRatioStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleDistribution"), crystal.polarAngleDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleAverage"), crystal.polarAngleAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleStd"), crystal.polarAngleStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.rotationDistribution"), crystal.rotationDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationAverage"), crystal.rotationAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationStd"), crystal.rotationStd);

    glDispatchCompute(finalNumGroups, 1, 1);
}

void SimulationEngine::Initialize()
{
    InitializeShader();
    InitializeTextures();
}

void SimulationEngine::InitializeShader()
{
    OpenGL::Shader shader(computeShaderSource, OpenGL::ShaderType::Compute);
    mSimulationShader = std::make_unique<OpenGL::Program>();
    try
    {
        shader.Compile();
        mSimulationShader->AttachShader(shader);
        mSimulationShader->Link();
    }
    catch (const std::runtime_error &)
    {
        throw;
    }
}

void SimulationEngine::InitializeTextures()
{
    mSimulationTexture = std::make_unique<OpenGL::Texture>(1920, 1080, 0, OpenGL::TextureType::Color);
    mSpinlockTexture = std::make_unique<OpenGL::Texture>(1920, 1080, 1, OpenGL::TextureType::Monochrome);
}

} // namespace HaloSim
