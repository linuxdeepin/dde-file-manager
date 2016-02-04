#include "dcheckablebutton.h"
#include <QLabel>
#include <QHBoxLayout>

DCheckableButton::DCheckableButton(const QString &icon, const QString &text, QWidget *parent):
    QPushButton(parent),
    m_icon(icon),
    m_text(text)
{
    setFlat(true);
    setCheckable(true);
    setObjectName("DCheckableButton");
    initUI();
}

DCheckableButton::~DCheckableButton()
{

}

void DCheckableButton::initUI()
{
    setFixedHeight(40);
    m_iconLabel = new QLabel;
    m_iconLabel->setPixmap(QPixmap(m_icon).scaledToHeight(30));
    m_iconLabel->setFixedSize(30, 30);
    m_textLabel = new QLabel;
    m_textLabel->setObjectName("LeftSideBarTextLabel");
    m_textLabel->setText(m_text);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textLabel->setFixedHeight(30);


    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(m_textLabel);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(20, 0, 0, 0);
    setLayout(mainLayout);
}

