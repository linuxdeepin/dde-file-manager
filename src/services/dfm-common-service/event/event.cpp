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

#include "event/protocol/dfmevent.h"
//#include "views/windowmanager.h"

#include <QDebug>
#include <QWidget>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QJsonArray>
#include <QMetaEnum>

DFMEvent::DFMEvent(int type):
    m_type(type),
    m_accept(false)
{

}

DFMEvent::DFMEvent(const DFMEvent &other)
    : m_type(other.m_type)
{

}

DFMEvent::~DFMEvent()
{

}

DFMEvent &DFMEvent::operator =(const DFMEvent &other)
{
    m_accept = other.m_accept;
    m_type = other.m_type;
    return *this;
}

void DFMEvent::setAccepted(bool accepted)
{
    m_accept = accepted;
}

bool DFMEvent::isAccepted() const
{
    return m_accept;
}

int DFMEvent::type() const
{
    return m_type;
}

void DFMEvent::setType(int type)
{
    m_type = type;
}
