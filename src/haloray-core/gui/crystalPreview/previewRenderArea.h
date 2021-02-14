#pragma once

#include <QWidget>
#include <QVector3D>
#include "../crystalModel.h"


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
    QVariant getFromModel(int row, CrystalModel::Columns column);
    CrystalModel *m_crystals;
    int m_populationIndex;
    QVector3D m_vertices[24];
};

}
