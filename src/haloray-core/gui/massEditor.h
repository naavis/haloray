#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>

namespace HaloRay
{

class MassEditor : public QDialog
{
    Q_OBJECT
public:
    explicit MassEditor(QWidget *parent = nullptr);

};

}
