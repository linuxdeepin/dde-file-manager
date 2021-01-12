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

#include "createtemplatefilejob.h"
#include "dbusadaptor/createtemplatefilejob_adaptor.h"

QString CreateTemplateFileJob::BaseObjectPath = "/com/deepin/filemanager/daemon/CreateTemplateFileJob";
QString CreateTemplateFileJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewCreateTempleFileJob";
int CreateTemplateFileJob::JobId = 0;

CreateTemplateFileJob::CreateTemplateFileJob(const QString &templateFile, const QString &targetDir, QObject *parent) :
    BaseJob(parent),
    m_templateFile(templateFile),
    m_targetDir(targetDir)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new CreateTemplateFileAdaptor(this);
}

CreateTemplateFileJob::~CreateTemplateFileJob()
{

}

void CreateTemplateFileJob::Execute()
{
    qDebug() << "CreateTemplateFileJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "CreateTemplateFileJob executing";
    }
    deleteLater();
}

