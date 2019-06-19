#pragma once
#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions_4_4_Core>
#include <memory>
#include "../simulation/simulationEngine.h"
#include "../opengl/textureRenderer.h"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_4_Core
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = 0);

protected:
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void initializeGL() override;

private:
    std::unique_ptr<HaloSim::SimulationEngine> mEngine;
    std::unique_ptr<OpenGL::TextureRenderer> mTextureRenderer;
};
