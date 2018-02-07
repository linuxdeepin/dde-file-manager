/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dstatebutton.h"

DStateButton::DStateButton(const QString &iconA, const QString &iconB, QWidget *parent):
    QPushButton(parent),
    m_aStateIcon(iconA),
    m_bStateIcon(iconB)
{
    m_buttonState = ButtonState::stateA;
    setIcon(QIcon(m_aStateIcon));
    setIconSize(QSize(16, 16));
    connect(this, SIGNAL(clicked()), this, SLOT(toogleStateICon()));
}

DStateButton::DStateButton(const QString &iconA, QWidget *parent):
    QPushButton(parent),
    m_aStateIcon(iconA)
{
    m_buttonState = ButtonState::stateA;
    setIcon(QIcon(m_aStateIcon));
    setIconSize(QSize(16, 16));
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
    setStyleSheet("QPushButton#SortingButton{background-color:white;\
                  border: 1px;\
                  color: black;\
                  border-radius: 2px; }");
}

void DStateButton::setBStateIcon()
{
    m_buttonState = ButtonState::stateB;
    setIcon(QIcon(m_bStateIcon));
    setStyleSheet("QPushButton#SortingButton{background-color:#2ca7f8;\
                  border: 1px;\
                  color: black;\
                  border-radius: 2px; }");
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

