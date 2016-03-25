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
    setFixedHeight(30);
    setFocusPolicy(Qt::NoFocus);
    QIcon icon(m_icon);
    m_iconLabel = new QLabel;
    m_iconLabel->setPixmap(icon.pixmap(16,16));
    m_iconLabel->setFixedSize(16, 16);
    m_textLabel = new QLabel;
    m_textLabel->setObjectName("LeftSideBarTextLabel");
    m_textLabel->setText(m_text);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textLabel->setFixedHeight(30);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(m_textLabel);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 0, 0, 0);
    setLayout(mainLayout);
}

