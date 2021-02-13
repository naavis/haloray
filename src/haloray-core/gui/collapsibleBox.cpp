#include "collapsibleBox.h"
#include <QPushButton>
#include <QVBoxLayout>

namespace HaloRay
{

CollapsibleBox::CollapsibleBox(QString title, QWidget *parent)
    : QWidget(parent)
{
    m_headerButton = new QPushButton(title);
    m_headerButton->setCheckable(true);

    m_content = new QWidget();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_headerButton);
    m_mainLayout->addWidget(m_content);

    connect(m_headerButton, &QPushButton::toggled, m_content, &QWidget::setVisible);
}

QWidget *CollapsibleBox::contentWidget() const
{
    return m_content;
}



}
