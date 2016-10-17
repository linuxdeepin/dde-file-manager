#include "dhoverbutton.h"
#include "dfilemenumanager.h"
#include "dfilemenu.h"
#include <QHBoxLayout>
#include <QDebug>

DHoverButton::DHoverButton(const QString &normal, const QString &hover, QWidget *parent)
    :QPushButton(parent)
{
    QIcon n(normal);
    m_normal = n;
    QIcon h(hover);
    m_hover = h;
    initUI();
}

void DHoverButton::setMenu(DFileMenu *menu)
{
    m_menu = menu;
}

void DHoverButton::enterEvent(QEvent *e)
{
    Q_UNUSED(e)

    m_iconLabel->setPixmap(m_hover.pixmap(16,16));
}

void DHoverButton::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)

    m_iconLabel->setPixmap(m_normal.pixmap(16,16));
}

void DHoverButton::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    if(m_menu)
        m_menu->exec(mapToGlobal(rect().bottomLeft()));
}

void DHoverButton::initUI()
{
    m_iconLabel = new QLabel;
    m_iconLabel->setPixmap(m_normal.pixmap(16,16));
    m_iconLabel->setFixedSize(16, 16);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconLabel);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}
