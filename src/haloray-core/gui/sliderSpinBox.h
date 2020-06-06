#pragma once
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QString;

namespace HaloRay
{

class SliderSpinBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue USER true)
public:
    SliderSpinBox(QWidget *parent = nullptr);
    SliderSpinBox(double min, double max, QWidget *parent = nullptr);
    void setSuffix(const QString &suffix);
    double value() const;

    static SliderSpinBox *createAngleSlider(double min, double max);

public slots:
    void setValue(double value);
    void setMinimum(double minimum);
    void setMaximum(double maximum);
    void setWrapping(bool wrapping);

signals:
    void valueChanged(double value);

private:
    QSlider *m_slider;
    QDoubleSpinBox *m_spinBox;
};

}
