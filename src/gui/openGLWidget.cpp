#include "openGLWidget.h"
#include <QWidget>
#include <QOpenGLWidget>
#include <memory>
#include "../simulation/simulationEngine.h"
#include "../simulation/camera.h"
#include "../simulation/lightSource.h"
#include "../simulation/crystalPopulation.h"
#include "../opengl/textureRenderer.h"

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      mDragging(false),
      mPreviousDragPoint(QPoint(0, 0)),
      mExposure(1.0f)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
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
    const float exposure = mExposure / (mEngine->GetIteration() + 1) / mEngine->GetCamera().fov;
    mTextureRenderer->SetUniformFloat("exposure", exposure);
    mTextureRenderer->Render(mEngine->GetOutputTextureHandle());
}

void OpenGLWidget::resizeGL(int w, int h)
{
    mEngine->ResizeOutputTextureCallback(w, h);
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

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (!mEngine->IsRunning())
        return;

    if (event->button() == Qt::LeftButton)
    {
        mDragging = true;
        this->setCursor(Qt::CursorShape::OpenHandCursor);
        mPreviousDragPoint = event->globalPos();
    }
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mDragging)
    {
        auto camera = mEngine->GetCamera();
        auto currentMousePosition = event->globalPos();
        auto delta = currentMousePosition - mPreviousDragPoint;
        camera.yaw += delta.x() * 0.2f * camera.fov;
        if (camera.yaw > 360.0)
            camera.yaw = -360.0;
        else if (camera.yaw < -360.0)
            camera.yaw = 360.0;

        camera.pitch += delta.y() * 0.2f * camera.fov;
        if (camera.pitch > 90.0)
            camera.pitch = 90.0;
        else if (camera.pitch < -90.0)
            camera.pitch = -90.0;
        mEngine->SetCamera(camera);

        emit cameraOrientationChanged(camera.pitch, camera.yaw);

        mPreviousDragPoint = currentMousePosition;
    }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mDragging = false;
        this->setCursor(Qt::CursorShape::ArrowCursor);
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    if (!mEngine->IsRunning())
    {
        event->ignore();
        return;
    }

    auto camera = mEngine->GetCamera();
    const float zoomSpeed = 0.1f * camera.fov;

    QPoint numPixels = event->pixelDelta();

    if (numPixels.isNull())
    {
        QPoint numDegrees = event->angleDelta() / 8.0;
        auto numSteps = numDegrees.y() / 15.0;
        camera.fov -= zoomSpeed * numSteps;
    }
    else
    {
        camera.fov -= zoomSpeed * numPixels.y();
    }

    camera.fov = std::max(camera.fov, 0.01f);
    camera.fov = std::min(camera.fov, 2.0f);
    mEngine->SetCamera(camera);

    event->accept();

    fieldOfViewChanged(camera.fov);
}

void OpenGLWidget::setBrightness(double brightness)
{
    mExposure = (float)brightness;
}
