#include "renderButton.h"
#include <QString>

RenderButton::RenderButton(QWidget *parent)
    : QPushButton(parent), mRendering(false)
{
    const QString renderText = "Render";
    const QString stopText = "Stop";

    this->setText(renderText);
    this->setStyleSheet("font: bold;");

    connect(this, &RenderButton::clicked, [this, renderText, stopText]() {
        if (mRendering)
            this->setText(renderText);
        else
            this->setText(stopText);
        mRendering = !mRendering;
    });
}
