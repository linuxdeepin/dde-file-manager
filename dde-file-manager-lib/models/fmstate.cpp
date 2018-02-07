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

#include "fmstate.h"

FMState::FMState(QObject *parent) : DebugObejct(parent)
{

}

FMState::~FMState()
{

}
int FMState::viewMode() const
{
    return m_viewMode;
}

void FMState::setViewMode(int viewMode)
{
    m_viewMode = viewMode;
}


int FMState::width() const
{
    return m_width;
}

void FMState::setWidth(int width)
{
    m_width = width;
}
int FMState::height() const
{
    return m_height;
}

void FMState::setHeight(int height)
{
    m_height = height;
}
int FMState::x() const
{
    return m_x;
}

void FMState::setX(int x)
{
    m_x = x;
}
int FMState::y() const
{
    return m_y;
}

void FMState::setY(int y)
{
    m_y = y;
}
int FMState::windowState() const
{
    return m_windowState;
}

void FMState::setWindowState(int windowState)
{
    m_windowState = windowState;
}

QString FMState::theme() const
{
    return m_theme;
}

void FMState::setTheme(const QString &theme)
{
    m_theme = theme;
}





