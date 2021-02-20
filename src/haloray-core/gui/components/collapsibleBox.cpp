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
    m_headerButton->setChecked(true);

    m_content = new QWidget();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_headerButton);
    m_mainLayout->addWidget(m_content);

    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(m_headerButton, &QPushButton::toggled, m_content, &QWidget::setVisible);
}

CollapsibleBox::CollapsibleBox(QString title, bool expanded, QWidget *parent)
    : CollapsibleBox(title, parent)
{
    m_headerButton->setChecked(expanded);
}

QWidget *CollapsibleBox::contentWidget() const
{
    return m_content;
}

}
