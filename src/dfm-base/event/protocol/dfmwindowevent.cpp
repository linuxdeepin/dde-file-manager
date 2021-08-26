/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "dfmwindowevent.h"

DFMWindowEvent::DFMWindowEvent(int type)
    :DFMEvent (type)
{

}

DFMWindowEvent &DFMWindowEvent::operator =(const DFMWindowEvent &event)
{
    DFMEvent::operator=(event);
    this->m_data = event.data();
    return * this;
}

void DFMWindowEvent::setData(QList<DFMWindowEventData> data) {
    m_data = data;
}

QList<DFMWindowEventData> DFMWindowEvent::data() const{
    return m_data;
}

QUrl DFMWindowEventData::target() const
{
    return m_target;
}

void DFMWindowEventData::setTarget(const QUrl &target)
{
    m_target = target;
}

DFMWindowEventData::DFMWindowEventData(int winIdx, const QUrl &target)
{
    m_winIdx = winIdx;
    m_target = target;
}

DFMWindowEventData::DFMWindowEventData(const DFMWindowEventData &data)
{
    m_winIdx = data.m_winIdx;
    m_target = data.m_target;
}

int DFMWindowEventData::winIdx() const
{
    return m_winIdx;
}

void DFMWindowEventData::setWinIdx(int winIdx)
{
    m_winIdx = winIdx;
}
