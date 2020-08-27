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

#include "historystack.h"

#include "dfileservices.h"
#include "plugins/pluginmanager.h"

#include <QDebug>

HistoryStack::HistoryStack(int threshold)
{
    m_threshold = threshold;
    m_index = -1;
}

void HistoryStack::append(DUrl url)
{
    if(m_index<=m_list.count()-1&&m_index>=0){
        if(m_list.at(m_index) == url)
            return;
    }

    if (m_index < m_threshold) {
        ++m_index;

        if (m_index != m_list.size()) {
            m_list = m_list.mid(0, m_index);
        }

        m_list.append(url);
    } else {
        m_list.takeFirst();
        m_list.append(url);
    }
}

DUrl HistoryStack::back()
{
    const DUrl &currentUrl = m_list.value(m_index);
    DUrl url;

    if (m_index <= 0)
        return url;

    while (--m_index >= 0) {
        if (m_index >= m_list.count())
            continue;

        url = m_list.at(m_index);

        if(url.isComputerFile())
            break;

        if(url.isUserShareFile())
            break;

        if (PluginManager::instance()->getViewInterfacesMap().keys().contains(url.scheme()))
            break;
        //判断网络文件是否可以到达
        if (DFileService::instance()->checkGvfsMountfileBusy(url,false)) {
            if (currentUrl == url) {
                removeAt(m_index);
                url = m_list.at(m_index);
            }
            else {
                break;
            }
        }
        else {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

            if (!fileInfo || !fileInfo->exists() || currentUrl == url) {
                removeAt(m_index);
                url = m_list.at(m_index);
            } else {
                break;
            }
        }
    }

    return url;
}

DUrl HistoryStack::forward()
{
    const DUrl &currentUrl = m_list.value(m_index);
    DUrl url;

    if (m_index >= m_list.count() - 1)
        return url;

    while (++m_index < m_list.count()) {
        url = m_list.at(m_index);

        if(url.isComputerFile())
            break;

        if(url.isUserShareFile())
            break;

        if (PluginManager::instance()->getViewInterfacesMap().keys().contains(url.scheme()))
            break;
        //判断网络文件是否可以到达
        if (DFileService::instance()->checkGvfsMountfileBusy(url,false)) {
            if (currentUrl == url) {
                removeAt(m_index);
                --m_index;
                url = m_list.at(m_index);
            }
            else {
                break;
            }
        }
        else {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

            if (!fileInfo || !fileInfo->exists() || currentUrl == url) {
                removeAt(m_index);
                --m_index;
                url = m_list.at(m_index);
            } else {
                break;
            }
        }
    }

    return url;
}

void HistoryStack::setThreshold(int threshold)
{
    m_threshold = threshold;
}

bool HistoryStack::isFirst()
{
    if(m_index < 0){
        m_index = 0;
    }
    return m_index == 0;
}

bool HistoryStack::isLast()
{
    if(m_index > m_list.size() - 1){
        m_index = m_list.size() - 1;
    }
    return m_index == m_list.size() - 1;
}

int HistoryStack::size()
{
    return m_list.size();
}

void HistoryStack::removeAt(int i)
{
    m_list.removeAt(i);
}

int HistoryStack::currentIndex()
{
    return m_index;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug beg, const HistoryStack &stack)
{
    beg << "history list:" << stack.m_list << "current pos:" << stack.m_index;

    return beg;
}
QT_END_NAMESPACE
