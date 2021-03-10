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

#include "deletejob.h"
#include "dbusadaptor/deletejob_adaptor.h"

QString DeleteJob::BaseObjectPath = "/com/deepin/filemanager/daemon/DeleteJob";
QString DeleteJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewDeleteJob";
int DeleteJob::JobId = 0;


DeleteJob::DeleteJob(const QStringList &filelist, QObject *parent) :
    BaseJob(parent),
    m_filelist(filelist)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new DeleteJobAdaptor(this);
}

DeleteJob::~DeleteJob()
{

}

void DeleteJob::Execute()
{
    qDebug() << "DeleteJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "DeleteJob executing";
    }
    deleteLater();
}

