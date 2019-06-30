#pragma once
#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QString>

class SliderSpinBox : public QWidget
{
    Q_OBJECT
public:
    SliderSpinBox(QWidget *parent = nullptr);
    void setSuffix(const QString &suffix);
    double value() const;

public slots:
    void setValue(double value);
    void setMinimum(double minimum);
    void setMaximum(double maximum);
    void setWrapping(bool wrapping);

signals:
    void valueChanged(double value);

private:
    QSlider *mSlider;
    QDoubleSpinBox *mSpinBox;
};
