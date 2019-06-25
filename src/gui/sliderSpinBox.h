#pragma once
#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>

class SliderSpinBox : public QWidget
{
    Q_OBJECT
public:
    SliderSpinBox(QWidget *parent = nullptr);

public slots:
    void setValue(double value);
    void setMinimum(double minimum);
    void setMaximum(double maximum);

signals:
    void valueChanged(double value);

private:
    QSlider *mSlider;
    QDoubleSpinBox *mSpinBox;
};
