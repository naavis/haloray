#pragma once

#include <QObject>
#include <QWidget>

class QComboBox;
class QCheckBox;

namespace HaloRay
{

class CrystalModel;
class PreviewRenderArea;

class CrystalPreviewWindow : public QWidget
{
    Q_OBJECT
public:
    CrystalPreviewWindow(CrystalModel *model, int initialIndex, QWidget *parent = nullptr);

public slots:
    void onMainWindowPopulationSelectionChange(int index);

private:
    void setupUi();

    CrystalModel *m_crystalModel;
    QComboBox *m_populationSelector;
    PreviewRenderArea *m_renderArea;
    QCheckBox *m_syncCheckBox;
    int m_selectedPopulationIndex;
};

}
