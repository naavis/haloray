#include "sliderSpinBox.h"
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>

namespace HaloRay {

const double sliderMultiplier = 100.0;

SliderSpinBox::SliderSpinBox(QWidget *parent) : QWidget(parent), m_value(0.0)
{
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Orientation::Horizontal);
    m_slider->setSingleStep((int)sliderMultiplier);
    m_slider->setPageStep((int)(10 * sliderMultiplier));
    m_slider->setMinimumWidth(100);

    m_spinBox = new QDoubleSpinBox();
    m_spinBox->setSingleStep(0.1);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(m_slider);
    layout->addWidget(m_spinBox);
    layout->setMargin(0);

    connect(m_slider, &QSlider::valueChanged, [this](int value) {
        setValue((double)value / sliderMultiplier);
    });
    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SliderSpinBox::setValue);
}

SliderSpinBox::SliderSpinBox(double min, double max, QWidget *parent)
    : SliderSpinBox(parent)
{
    setMinimum(min);
    setMaximum(max);
}

void SliderSpinBox::setSuffix(const QString &suffix)
{
    m_spinBox->setSuffix(suffix);
}

void SliderSpinBox::setMinimum(double minimum)
{
    m_slider->setMinimum((int)(minimum * sliderMultiplier));
    m_spinBox->setMinimum(minimum);
    if (m_value < minimum) setValue(minimum);
}

void SliderSpinBox::setMaximum(double maximum)
{
    m_slider->setMaximum((int)(maximum * sliderMultiplier));
    m_spinBox->setMaximum(maximum);
    if (m_value > maximum) setValue(maximum);
    emit maximumChanged(maximum);
}

void SliderSpinBox::setWrapping(bool wrapping)
{
    m_spinBox->setWrapping(wrapping);
}

void SliderSpinBox::setValue(double value)
{
    if (value == m_value) return;
    m_spinBox->setValue(value);
    m_slider->setValue((int)(value * sliderMultiplier));
    m_value = value;
    emit valueChanged(m_value);
}

double SliderSpinBox::value() const
{
    return m_value;
}

double SliderSpinBox::maximum() const
{
    return m_spinBox->maximum();
}

SliderSpinBox *SliderSpinBox::createAngleSlider(double min, double max)
{
    auto slider = new SliderSpinBox(min, max);
    slider->setSuffix("°");
    return slider;
}

}
