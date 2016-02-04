#include "dstatebutton.h"

DStateButton::DStateButton(const QString &iconA, const QString &iconB, QWidget *parent):
    QPushButton(parent),
    m_aStateIcon(iconA),
    m_bStateIcon(iconB)
{
    m_buttonState = ButtonState::stateA;
    setIcon(QIcon(m_aStateIcon));
    setIconSize(QSize(30, 30));
    connect(this, SIGNAL(clicked()), this, SLOT(toogleStateICon()));
}

DStateButton::DStateButton(const QString &iconA, QWidget *parent):
    QPushButton(parent),
    m_aStateIcon(iconA)
{
    m_buttonState = ButtonState::stateA;
    setIcon(QIcon(m_aStateIcon));
    setIconSize(QSize(30, 30));
}

DStateButton::~DStateButton()
{

}

DStateButton::ButtonState DStateButton::getButtonState()
{
    return m_buttonState;
}

void DStateButton::setAStateIcon()
{
    m_buttonState = ButtonState::stateA;
    setIcon(QIcon(m_aStateIcon));
}

void DStateButton::setBStateIcon()
{
    m_buttonState = ButtonState::stateB;
    setIcon(QIcon(m_bStateIcon));
}

void DStateButton::toogleStateICon()
{
    if (m_buttonState == ButtonState::stateA){
        setBStateIcon();
    }else{
        setAStateIcon();
    }
}

void DStateButton::setButtonState(DStateButton::ButtonState state)
{
    if (state == ButtonState::stateA){
        setAStateIcon();
    }else{
        setBStateIcon();
    }
}

