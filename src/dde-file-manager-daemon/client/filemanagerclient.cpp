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

#include "filemanagerclient.h"
#include "app/global.h"
#include "dbusservice/dbustype/dbusinforet.h"
#include "dbusservice/dbusinterface/renamejob_interface.h"
#include <QDBusConnection>
#include <QDebug>

FileManagerClient::FileManagerClient(QObject *parent) : QObject(parent)
{
    DBusInfoRet::registerMetaType();
    QString objectPath = "/com/deepin/filemanager/daemon/Operations";
    m_fileOperationInterface = new FileOperationInterface(DaemonServicePath, objectPath, QDBusConnection::systemBus());
    testNewNameJob("11111", "222222222");
}

FileManagerClient::~FileManagerClient()
{

}

void FileManagerClient::testNewNameJob(const QString &oldFile, const QString &newFile)
{
    QDBusPendingReply<DBusInfoRet> reply = m_fileOperationInterface->NewRenameJob(oldFile, newFile);
    reply.waitForFinished();
    if (reply.isFinished()){
        DBusInfoRet result = qdbus_cast<DBusInfoRet>(reply.argumentAt(0));
        qDebug() << result;
        RenameJobInterface* renamejobInterface = new RenameJobInterface(result.ServicePath, result.InterfacePath.path(), QDBusConnection::systemBus());
        renamejobInterface->Execute();
    }else{

    }
}

