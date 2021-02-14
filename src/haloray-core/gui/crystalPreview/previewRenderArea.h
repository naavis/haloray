#pragma once

#include <QWidget>

class QVector3D;

namespace HaloRay
{

class PreviewRenderArea : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewRenderArea(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    static void initializeGeometry(QVector3D *vertices, int numVertices);
};

}
