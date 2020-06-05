#include "openGLWidget.h"
#include <QWidget>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <memory>
#include <algorithm>
#include "../simulation/simulationEngine.h"
#include "../simulation/camera.h"
#include "../simulation/lightSource.h"
#include "../simulation/crystalPopulation.h"
#include "../opengl/textureRenderer.h"

OpenGLWidget::OpenGLWidget(enginePtr engine, QWidget *parent)
    : QOpenGLWidget(parent),
      mEngine(engine),
      mDragging(false),
      mPreviousDragPoint(QPoint(0, 0)),
      mExposure(1.0f),
      mMaxIterations(1)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setUpdateBehavior(UpdateBehavior::PartialUpdate);
}

void OpenGLWidget::toggleRendering()
{
    if (mEngine->isRunning())
        mEngine->stop();
    else
        mEngine->start();
    update();
}

void OpenGLWidget::paintGL()
{
    if (mEngine->isRunning() && mEngine->getIteration() < mMaxIterations)
    {
        mEngine->step();
        emit nextIteration(mEngine->getIteration());
        update();
    }
    const float exposure = mExposure / (mEngine->getIteration() + 1) / (mEngine->getCamera().fov / 180.0);
    mTextureRenderer->setUniformFloat("exposure", exposure);
    mTextureRenderer->render(mEngine->getOutputTextureHandle());
}

void OpenGLWidget::resizeGL(int w, int h)
{
    mEngine->resizeOutputTextureCallback(w, h);
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    mTextureRenderer = std::make_unique<OpenGL::TextureRenderer>();
    mEngine->initialize();
    mTextureRenderer->initialize();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    int maxComputeGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeGroups);
    const int absoluteMaxRaysPerFrame = 5000000;
    int maxRaysPerFrame = std::min(absoluteMaxRaysPerFrame, maxComputeGroups);
    emit maxRaysPerFrameChanged(maxRaysPerFrame);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (!mEngine->isRunning())
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
        auto camera = mEngine->getCamera();
        auto dragSpeed = camera.fov / height();
        auto currentMousePosition = event->globalPos();
        auto delta = currentMousePosition - mPreviousDragPoint;
        camera.yaw += delta.x() * dragSpeed;
        if (camera.yaw > 360.0)
            camera.yaw = -360.0;
        else if (camera.yaw < -360.0)
            camera.yaw = 360.0;

        camera.pitch += delta.y() * dragSpeed;
        if (camera.pitch > 90.0)
            camera.pitch = 90.0;
        else if (camera.pitch < -90.0)
            camera.pitch = -90.0;
        mEngine->setCamera(camera);

        emit cameraOrientationChanged(camera.pitch, camera.yaw);

        mPreviousDragPoint = currentMousePosition;
        update();
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
    if (!mEngine->isRunning())
    {
        event->ignore();
        return;
    }

    auto camera = mEngine->getCamera();
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
        camera.fov -= 0.01 * zoomSpeed * numPixels.y();
    }

    camera.fov = std::max(camera.fov, 10.0f);
    camera.fov = std::min(camera.fov, camera.getMaximumFov());
    mEngine->setCamera(camera);

    event->accept();

    fieldOfViewChanged(camera.fov);
    update();
}

void OpenGLWidget::setBrightness(double brightness)
{
    mExposure = (float)brightness;
    update();
}

void OpenGLWidget::setMaxIterations(unsigned int maxIterations)
{
    mMaxIterations = maxIterations;
    update();
}

QSize OpenGLWidget::sizeHint() const
{
    return QSize(800, 600);
}

QSize OpenGLWidget::minimumSizeHint() const
{
    return QSize(320, 320);
}
