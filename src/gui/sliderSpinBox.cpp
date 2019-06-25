#include "sliderSpinBox.h"
#include <QHBoxLayout>

SliderSpinBox::SliderSpinBox(QWidget *parent) : QWidget(parent)
{
    mSlider = new QSlider(this);
    mSlider->setOrientation(Qt::Orientation::Horizontal);
    mSpinBox = new QDoubleSpinBox(this);

    connect(mSlider, &QSlider::valueChanged, [=](int value) {
        mSpinBox->setValue((double)value);
    });
    connect(mSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        mSlider->setValue((int)value);
    });
    connect(mSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SliderSpinBox::valueChanged);

    auto layout = new QHBoxLayout();
    layout->addWidget(mSlider);
    layout->addWidget(mSpinBox);
    setLayout(layout);
}

void SliderSpinBox::setMinimum(double minimum)
{
    mSlider->setMinimum((int)minimum);
    mSpinBox->setMinimum(minimum);
}

void SliderSpinBox::setMaximum(double maximum)
{
    mSlider->setMaximum((int)maximum);
    mSpinBox->setMaximum(maximum);
}

void SliderSpinBox::setValue(double value)
{
    mSpinBox->setValue(value);
}
