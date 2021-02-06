#include "openGLWidget.h"
#include <QWidget>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <memory>
#include <algorithm>
#include "simulationStateModel.h"
#include "../simulation/simulationEngine.h"
#include "../simulation/camera.h"
#include "../simulation/lightSource.h"
#include "../simulation/crystalPopulation.h"

namespace HaloRay
{

OpenGLWidget::OpenGLWidget(SimulationEngine *engine, SimulationStateModel *viewModel, QWidget *parent)
    : QOpenGLWidget(parent),
      m_engine(engine),
      m_dragging(false),
      m_previousDragPoint(QPoint(0, 0)),
      m_exposure(1.0f),
      m_viewModel(viewModel)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setUpdateBehavior(UpdateBehavior::PartialUpdate);
    connect(m_viewModel, &SimulationStateModel::dataChanged, [this]() {
        update();
    });
}

void OpenGLWidget::toggleRendering()
{
    if (m_engine->isRunning())
        m_engine->stop();
    else
        m_engine->start();
    update();
}

void OpenGLWidget::paintGL()
{
    if (m_engine->isRunning() && m_engine->getIteration() < m_viewModel->getMaxIterations())
    {
        m_engine->step();
        emit nextIteration(m_engine->getIteration());
        update();
    }
    const float adjustedExposure = m_exposure / (m_engine->getIteration() + 1) / (m_engine->getCamera().fov / 180.0);
    m_textureRenderer->setUniformFloat("adjustedExposure", adjustedExposure);
    m_textureRenderer->setUniformFloat("baseExposure", m_exposure);
    m_textureRenderer->render(m_engine->getOutputTextureHandle(), m_engine->getBackgroundTextureHandle());
}

void OpenGLWidget::resizeGL(int w, int h)
{
    m_engine->resizeOutputTextureCallback(w, h);
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_textureRenderer = std::make_unique<OpenGL::TextureRenderer>();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    int maxComputeGroups;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeGroups);
    const int absoluteMaxRaysPerFrame = 5000000;
    int maxRaysPerFrame = std::min(absoluteMaxRaysPerFrame, maxComputeGroups);
    m_viewModel->setRaysPerFrameUpperLimit(maxRaysPerFrame);
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_engine->isRunning())
        return;

    if (event->button() == Qt::LeftButton)
    {
        m_dragging = true;
        this->setCursor(Qt::CursorShape::OpenHandCursor);
        m_previousDragPoint = event->globalPos();
    }
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging)
    {
        auto camera = m_engine->getCamera();
        auto dragSpeed = camera.fov / height();
        auto currentMousePosition = event->globalPos();
        auto delta = currentMousePosition - m_previousDragPoint;
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
        m_engine->setCamera(camera);

        emit cameraOrientationChanged(camera.pitch, camera.yaw);

        m_previousDragPoint = currentMousePosition;
    }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        this->setCursor(Qt::CursorShape::ArrowCursor);
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_engine->isRunning())
    {
        event->ignore();
        return;
    }

    auto camera = m_engine->getCamera();
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

    camera.fov = std::max(camera.fov, 1.5f);
    camera.fov = std::min(camera.fov, camera.getMaximumFov());
    m_engine->setCamera(camera);

    event->accept();

    fieldOfViewChanged(camera.fov);
}

void OpenGLWidget::setBrightness(double brightness)
{
    m_exposure = (float)brightness;
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

}
