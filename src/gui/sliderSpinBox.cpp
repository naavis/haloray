#include "sliderSpinBox.h"
#include <QHBoxLayout>

const double sliderMultiplier = 100.0;

SliderSpinBox::SliderSpinBox(QWidget *parent) : QWidget(parent)
{
    mSlider = new QSlider(this);
    mSlider->setOrientation(Qt::Orientation::Horizontal);
    mSlider->setSingleStep((int)sliderMultiplier);
    mSlider->setPageStep((int)(10 * sliderMultiplier));

    mSpinBox = new QDoubleSpinBox(this);
    mSpinBox->setSingleStep(0.1);

    auto layout = new QHBoxLayout();
    layout->addWidget(mSlider);
    layout->addWidget(mSpinBox);
    setLayout(layout);

    setMinimumWidth(400);

    connect(mSlider, &QSlider::valueChanged, [=](int value) {
        mSpinBox->setValue((double)value / sliderMultiplier);
    });
    connect(mSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        mSlider->setValue((int)(value * sliderMultiplier));
    });
    connect(mSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SliderSpinBox::valueChanged);
}

void SliderSpinBox::setSuffix(const QString &suffix)
{
    mSpinBox->setSuffix(suffix);
}

void SliderSpinBox::setMinimum(double minimum)
{
    mSlider->setMinimum((int)(minimum * sliderMultiplier));
    mSpinBox->setMinimum(minimum);
}

void SliderSpinBox::setMaximum(double maximum)
{
    mSlider->setMaximum((int)(maximum * sliderMultiplier));
    mSpinBox->setMaximum(maximum);
}

void SliderSpinBox::setValue(double value)
{
    mSpinBox->setValue(value);
}

double SliderSpinBox::value() const
{
    return mSpinBox->value();
}
