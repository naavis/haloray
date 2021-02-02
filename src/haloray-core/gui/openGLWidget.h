#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_4_Core>
#include <memory>
#include "../opengl/textureRenderer.h"


class QMouseEvent;
class QWheelEvent;
class QSize;
class QPoint;

namespace HaloRay
{
class SimulationEngine;
class SimulationStateModel;

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_4_Core
{
    Q_OBJECT

public:
    explicit OpenGLWidget(SimulationEngine *engine, SimulationStateModel *viewModel, QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void toggleRendering();
    void setBrightness(double brightness);

signals:
    void fieldOfViewChanged(double fieldOfView);
    void cameraOrientationChanged(double pitch, double yaw);
    void nextIteration(unsigned int iteration);

protected:
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void initializeGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    SimulationEngine  *m_engine;
    std::unique_ptr<OpenGL::TextureRenderer> m_textureRenderer;
    bool m_dragging;
    QPoint m_previousDragPoint;
    float m_exposure;
    SimulationStateModel *m_viewModel;
};

}
