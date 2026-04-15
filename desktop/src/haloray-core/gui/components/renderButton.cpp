#include "renderButton.h"
#include <QString>

namespace HaloRay
{

RenderButton::RenderButton(QWidget *parent)
    : QPushButton(parent), m_rendering(false)
{
    const QString renderText = tr("Render");
    const QString stopText = tr("Stop");

    setText(renderText);
    setStyleSheet("font: bold;");
    setMinimumHeight(100);

    connect(this, &RenderButton::clicked, [this, renderText, stopText]() {
        if (m_rendering)
            this->setText(renderText);
        else
            this->setText(stopText);
        m_rendering = !m_rendering;
    });
}

}
