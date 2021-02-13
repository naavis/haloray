#pragma once

#include <QObject>
#include <QWidget>

class QComboBox;

namespace HaloRay
{

class CrystalModel;

class CrystalPreviewWindow : public QWidget
{
    Q_OBJECT
public:
    CrystalPreviewWindow(CrystalModel *model, QWidget *parent = nullptr);

private:
    void setupUi();

    CrystalModel *m_crystalModel;
    QComboBox *m_populationSelector;
};

}
