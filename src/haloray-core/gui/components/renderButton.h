#pragma once
#include <QPushButton>

namespace HaloRay
{

class RenderButton : public QPushButton
{
    Q_OBJECT
public:
    RenderButton(QWidget *parent = nullptr);

private:
    bool m_rendering;
};

}
