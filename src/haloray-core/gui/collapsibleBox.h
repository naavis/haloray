#pragma once

#include <QObject>
#include <QWidget>

class QPushButton;
class QVBoxLayout;

namespace HaloRay
{

class CollapsibleBox : public QWidget
{
    Q_OBJECT
public:
    CollapsibleBox(QString title, QWidget *parent = nullptr);

private:
    explicit CollapsibleBox(QWidget *) {};

    QPushButton *m_headerButton;
    QVBoxLayout *m_mainLayout;
    QWidget *m_content;
};

}
