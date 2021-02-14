#include "previewRenderArea.h"
#include <QPainter>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

namespace HaloRay
{

PreviewRenderArea::PreviewRenderArea(QWidget *parent) : QWidget(parent)
{
    // TODO: Set size policy and size hint
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PreviewRenderArea::paintEvent(QPaintEvent *)
{
    // Example cube vertices
    QVector3D vertices[] =
    {
        QVector3D(-1.0f, -1.0f, -1.0f),
        QVector3D(1.0f, -1.0f, -1.0f),
        QVector3D(1.0f, 1.0f, -1.0f),
        QVector3D(-1.0f, 1.0f, -1.0f),
        QVector3D(-1.0f, -1.0f, 1.0f),
        QVector3D(1.0f, -1.0f, 1.0f),
        QVector3D(1.0f, 1.0f, 1.0f),
        QVector3D(-1.0f, 1.0f, 1.0f),
    };

    QMatrix4x4 transformMat;
    transformMat.perspective(45.0f, 1.0f, 0.01f, 10000.0f);
    transformMat.scale(50.0f);
    transformMat.lookAt(QVector3D(2.0f, 2.0f, 3.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    QPen vertexPen;
    vertexPen.setWidth(5);
    vertexPen.setColor(QColor(255, 0, 0));
    painter.setPen(vertexPen);

    painter.translate(width() / 2, height() / 2);
    int side = qMin(width(), height());
    painter.scale(side / 500.0, side / 500.0);


    QVector4D mappedVertices[8];
    for (int i = 0; i < 8; ++i)
    {
        mappedVertices[i] = transformMat * vertices[i].toVector4D();
    }

    for (int i = 0; i < 8; ++i)
    {
        painter.drawPoint(mappedVertices[i].x(), mappedVertices[i].y());
    }

    QPen edgePen;
    edgePen.setWidth(3);
    edgePen.setColor(QColor(0, 255, 0));
    painter.setPen(edgePen);
    for (int i = 1; i < 8; ++i)
    {
        painter.drawLine(mappedVertices[i - 1].x(), mappedVertices[i-1].y(), mappedVertices[i].x(), mappedVertices[i].y());
    }
}

}
