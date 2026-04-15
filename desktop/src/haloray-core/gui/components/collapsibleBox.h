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
    CollapsibleBox(QString title, bool expanded, QWidget * parent = nullptr);

    QWidget *contentWidget() const;

private:
    explicit CollapsibleBox(QWidget *) {};

    QPushButton *m_headerButton;
    QVBoxLayout *m_mainLayout;
    QWidget *m_content;
};

}
