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

#include "fileoperation.h"
#include "dbusservice/dbusadaptor/fileoperation_adaptor.h"

#include "fileoperationjob/copyjob.h"
#include "fileoperationjob/movejob.h"
#include "fileoperationjob/createfolderjob.h"
#include "fileoperationjob/createtemplatefilejob.h"
#include "fileoperationjob/renamejob.h"
#include "fileoperationjob/deletejob.h"

#include <QDBusConnection>
#include <QDBusVariant>
#include <QDebug>

QString FileOperation::ObjectPath = "/com/deepin/filemanager/daemon/Operations";

FileOperation::FileOperation(const QString &servicePath, QObject *parent) :
    QObject(parent)
{
    DBusInfoRet::registerMetaType();
    m_servicePath = servicePath;
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_fileOperationAdaptor = new FileOperationAdaptor(this);
}

FileOperation::~FileOperation()
{

}

DBusInfoRet FileOperation::NewCreateFolderJob(const QString &fabspath)
{
    DBusInfoRet result;
    qDebug() << fabspath;
    CreateFolderJob* job = new CreateFolderJob(fabspath);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewCreateTemplateFileJob(const QString &templateFile, const QString &targetDir)
{
    DBusInfoRet result;
    qDebug() << templateFile << targetDir;
    CreateTemplateFileJob* job = new CreateTemplateFileJob(templateFile, targetDir);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewCopyJob(const QStringList &filelist, const QString &targetDir)
{
    DBusInfoRet result;
    qDebug() << filelist << targetDir;
    CopyJob* job = new CopyJob(filelist, targetDir);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewMoveJob(const QStringList &filelist, const QString &targetDir)
{
    DBusInfoRet result;
    qDebug() << filelist << targetDir;
    MoveJob* job = new MoveJob(filelist, targetDir);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewRenameJob(const QString &oldFile, const QString &newFile)
{
    DBusInfoRet result;
    qDebug() << oldFile << newFile << QDBusConnection::systemBus().name();
    RenameJob* job = new RenameJob(oldFile, newFile);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewDeleteJob(const QStringList &filelist)
{
    DBusInfoRet result;
    qDebug() << filelist;
    DeleteJob* job = new DeleteJob(filelist);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

QString FileOperation::test(const QString &oldFile, const QString &newFile, QDBusObjectPath &result2, bool &result3)
{
    Q_UNUSED(oldFile)
    Q_UNUSED(newFile)

    result2.setPath("m_renameJobBasePath");
    result3 = true;

    qDebug() << result2.path() << result3;

    return "1111111111111";
}
