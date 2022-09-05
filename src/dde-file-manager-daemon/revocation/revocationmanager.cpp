// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

void RevocationManager::pushEvent(int event, const QString & user)
{
    if (REVOCATION_TIMES == m_eventStack.size()) {
        m_eventStack.pop_front();
    }
    RevocationEvent evtTypeUser;
    evtTypeUser.eventType = static_cast<RevocationEventType>(event);
    evtTypeUser.user = user;
    m_eventStack.push(evtTypeUser);
}

int RevocationManager::popEvent()
{
    if (m_eventStack.isEmpty())
        return DFM_NO_EVENT;

    RevocationEvent event = m_eventStack.pop();
    switch (event.eventType) {
    case DFM_FILE_MGR:
        emit fmgrRevocationAction(event.user);
        break;
    case DFM_DESKTOP:
        emit deskRevocationAction(event.user);
        break;
    case DFM_NO_EVENT:
        break;
    }
    return event.eventType;
}
