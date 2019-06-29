#include "openGLWidget.h"
#include <QWidget>
#include <QOpenGLWidget>
#include <memory>
#include "../simulation/simulationEngine.h"
#include "../simulation/camera.h"
#include "../simulation/lightSource.h"
#include "../simulation/crystalPopulation.h"
#include "../opengl/textureRenderer.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}

void OpenGLWidget::setEngine(enginePtr engine)
{
    mEngine = engine;
}

void OpenGLWidget::toggleRendering()
{
    if (mEngine->IsRunning())
        mEngine->Stop();
    else
        mEngine->Start();
    update();
}

void OpenGLWidget::paintGL()
{
    if (mEngine->IsRunning())
    {
        mEngine->Step();
        update();
    }
    const float exposure = 1.0f / (mEngine->GetIteration() + 1) / mEngine->GetCamera().fov;
    mTextureRenderer->SetUniformFloat("exposure", exposure);
    mTextureRenderer->Render(mEngine->GetOutputTextureHandle());
}

void OpenGLWidget::resizeGL(int w, int h)
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    mTextureRenderer = std::make_unique<OpenGL::TextureRenderer>();
    mEngine->Initialize();
    mTextureRenderer->Initialize();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}
