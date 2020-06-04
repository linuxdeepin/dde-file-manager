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

#include <QProcess>

#include "dbusfilemanager1.h"
#include "dfilewatcher.h"
#include "dfileservices.h"
#include "app/define.h"
#include "dialogs/dialogmanager.h"
#include "../dialogs/dtaskdialog.h"


#include <QDebug>

DBusFileManager1::DBusFileManager1(QObject *parent)
    : QObject(parent)
{

}

void DBusFileManager1::ShowFolders(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--raw" << URIs))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--raw" << URIs);
}

void DBusFileManager1::ShowItemProperties(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--raw" << "-p" << URIs))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--raw" << "-p" << URIs);
}

void DBusFileManager1::ShowItems(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--show-item" <<  URIs << "--raw"))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" <<  URIs << "--raw");
}

void DBusFileManager1::Trash(const QStringList &URIs)
{
    DUrlList urls;
    foreach (const QString& path, URIs) {
        DUrl url = DUrl::fromUserInput(path);
        urls << url;
    }
   DFileService::instance()->moveToTrash(this, urls);

}

QStringList DBusFileManager1::GetMonitorFiles() const
{
    return DFileWatcher::getMonitorFiles();
}

bool DBusFileManager1::topTaskDialog()
{
    // 如果正在有保险箱的移动、粘贴操作，置顶弹出任务框
    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if(pTaskDlg){
        if(pTaskDlg->bHaveNotCompletedVaultTask()){
            pTaskDlg->showDialogOnTop();
            return true;
        }
    }
    return false;
}

void DBusFileManager1::closeTask()
{
    // 如果正在有保险箱的移动、粘贴、删除操作，强行结束任务
    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if(pTaskDlg){
        if(pTaskDlg->bHaveNotCompletedVaultTask()){
            pTaskDlg->stopVaultTask();
        }
    }
}
