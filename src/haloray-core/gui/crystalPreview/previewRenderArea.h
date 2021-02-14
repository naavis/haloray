#pragma once

#include <QWidget>

namespace HaloRay
{

class PreviewRenderArea : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewRenderArea(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}
