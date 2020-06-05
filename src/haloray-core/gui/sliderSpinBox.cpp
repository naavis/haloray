#include "sliderSpinBox.h"
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>

namespace HaloRay {

const double sliderMultiplier = 100.0;

SliderSpinBox::SliderSpinBox(QWidget *parent) : QWidget(parent)
{
    mSlider = new QSlider();
    mSlider->setOrientation(Qt::Orientation::Horizontal);
    mSlider->setSingleStep((int)sliderMultiplier);
    mSlider->setPageStep((int)(10 * sliderMultiplier));
    mSlider->setMinimumWidth(100);

    mSpinBox = new QDoubleSpinBox();
    mSpinBox->setSingleStep(0.1);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(mSlider);
    layout->addWidget(mSpinBox);
    layout->setMargin(0);

    connect(mSlider, &QSlider::valueChanged, [this](int value) {
        mSpinBox->setValue((double)value / sliderMultiplier);
    });
    connect(mSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        mSlider->setValue((int)(value * sliderMultiplier));
    });
    connect(mSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SliderSpinBox::valueChanged);
}

SliderSpinBox::SliderSpinBox(double min, double max, QWidget *parent)
    : SliderSpinBox(parent)
{
    setMinimum(min);
    setMaximum(max);
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

void SliderSpinBox::setWrapping(bool wrapping)
{
    mSpinBox->setWrapping(wrapping);
}

void SliderSpinBox::setValue(double value)
{
    if (mSpinBox->value() == value) return;
    mSpinBox->setValue(value);
}

double SliderSpinBox::value() const
{
    return mSpinBox->value();
}

SliderSpinBox *SliderSpinBox::createAngleSlider(double min, double max)
{
    auto slider = new SliderSpinBox(min, max);
    slider->setSuffix("°");
    return slider;
}

}
