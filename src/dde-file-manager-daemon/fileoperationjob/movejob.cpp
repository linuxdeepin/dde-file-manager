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

#include "movejob.h"
#include "dbusadaptor/movejob_adaptor.h"

QString MoveJob::BaseObjectPath = "/com/deepin/filemanager/daemon/MoveJob";
QString MoveJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewMoveJob";
int MoveJob::JobId = 0;

MoveJob::MoveJob(const QStringList &filelist, const QString &targetDir, QObject *parent) :
    BaseJob(parent),
    m_filelist(filelist),
    m_targetDir(targetDir)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new MoveJobAdaptor(this);
}

MoveJob::~MoveJob()
{

}

void MoveJob::Execute()
{
    qDebug() << "MoveJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "MoveJob executing";
    }
    deleteLater();
}

