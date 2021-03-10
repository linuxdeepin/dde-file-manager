/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "copyjob.h"
#include "dbusadaptor/copyjob_adaptor.h"

QString CopyJob::BaseObjectPath = "/com/deepin/filemanager/daemon/CreateCopyJob";
QString CopyJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewCopyJob";
int CopyJob::JobId = 0;

CopyJob::CopyJob(const QStringList &filelist, const QString &targetDir, QObject *parent) :
    BaseJob(parent),
    m_filelist(filelist),
    m_targetDir(targetDir)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new CopyJobAdaptor(this);
}

CopyJob::~CopyJob()
{

}

void CopyJob::Execute()
{
    qDebug() << "CreateFolderJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "CreateFolderJob executing";
    }
    deleteLater();
}
