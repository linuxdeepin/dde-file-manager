/*
 * Copyright (C) 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Lu Zhen <luzhen@uniontech.com>
 *
 * Maintainer: Lu Zhen <luzhen@uniontech.com>
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

#include "revocationmanager.h"
#include "dfmglobal.h"
#include <QDBusConnection>
#include "dbusadaptor/revocationmgr_adaptor.h"

QString RevocationManager::ObjectPath = "/com/deepin/filemanager/daemon/RevocationManager";

RevocationManager::RevocationManager(QObject *parent)
    : QObject(parent)
    , QDBusContext()
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_adaptor = new RevocationMgrAdaptor(this);
}

RevocationManager::~RevocationManager()
{
}

void RevocationManager::pushEvent(int event)
{
    if (REVOCATION_TIMES == m_eventStack.size()) {
        m_eventStack.pop_front();
    }

    m_eventStack.push(static_cast<RevocationEventType>(event));
}

int RevocationManager::popEvent()
{
    if (m_eventStack.isEmpty())
        return DFM_NO_EVENT;

    RevocationEventType event = m_eventStack.pop();
    switch (event) {
    case DFM_FILE_MGR:
        emit fmgrRevocationAction();
        break;
    case DFM_DESKTOP:
        emit deskRevocationAction();
        break;
    case DFM_NO_EVENT:
        break;
    }
    return event;
}
