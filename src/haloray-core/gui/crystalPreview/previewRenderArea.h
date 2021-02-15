#pragma once

#include <QWidget>
#include <QVector3D>
#include "../crystalModel.h"

class QMatrix4x4;

namespace HaloRay
{

class PreviewRenderArea : public QWidget
{
    Q_OBJECT
public:
    PreviewRenderArea(CrystalModel *crystals, QWidget *parent = nullptr);

public slots:
    void onPopulationSelectionChange(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void initializeGeometry(QVector3D *vertices, int numVertices);

private:
    QVariant getFromModel(int row, CrystalModel::Columns column) const;
    QMatrix4x4 getCrystalOrientationMatrix() const;
    float getFurthestVertexDistance(QVector3D *vertices, int numVertices) const;

    CrystalModel *m_crystals;
    int m_populationIndex;
    QVector3D m_vertices[24];
};

}
