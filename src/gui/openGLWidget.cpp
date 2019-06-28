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
    {
        mEngine->Stop();
        update();
    }
    else
    {
        mEngine->Start(500000);
        update();
    }
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

    HaloSim::Camera camera;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.fov = 0.5f;
    camera.projection = HaloSim::Projection::Stereographic;
    camera.hideSubHorizon = false;

    HaloSim::LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;

    HaloSim::CrystalPopulation crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.tiltDistribution = 1;
    crystalProperties.tiltAverage = 0.0f;
    crystalProperties.tiltStd = 40.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 0.0f;
    crystalProperties.rotationStd = 1.0f;

    mEngine->SetCamera(camera);
    mEngine->SetLightSource(sun);
    mEngine->SetCrystalPopulation(crystalProperties);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}
