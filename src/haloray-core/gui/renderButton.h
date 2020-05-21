#pragma once
#include <QPushButton>
#include <QWidget>

class RenderButton : public QPushButton
{
    Q_OBJECT
public:
    RenderButton(QWidget *parent = nullptr);

private:
    bool mRendering;
};
