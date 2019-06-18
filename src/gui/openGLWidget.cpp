#include "openGLWidget.h"
#include <QWidget>
#include <QOpenGLWidget>

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}

void OpenGLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWidget::resizeGL(int w, int h)
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}
