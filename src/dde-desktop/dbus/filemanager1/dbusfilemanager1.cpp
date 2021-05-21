/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#include <unistd.h>
#include <QProcess>
#include "dbusfilemanager1.h"
#include "dfilewatcher.h"
#include "dfileservices.h"
#include "app/define.h"
#include "dialogs/dialogmanager.h"
#include "../dialogs/dtaskdialog.h"
#include "../dde-file-manager-lib/vault/vaultglobaldefine.h"


#include <QDebug>

DBusFileManager1::DBusFileManager1(QObject *parent)
    : QObject(parent)
{
    // monitor screen lock event.
    QDBusConnection::sessionBus().connect(
        "com.deepin.SessionManager",
        "/com/deepin/SessionManager",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged", "sa{sv}as",
        this,
        SLOT(lockPropertyChanged(QDBusMessage)));
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
    foreach (const QString &path, URIs) {
        DUrl tempUrl = DUrl::fromUserInput(path);
        urls << tempUrl;
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
    if (pTaskDlg && pTaskDlg->haveNotCompletedVaultTask()) {
        pTaskDlg->showDialogOnTop();
        return true;
    }
    return false;
}

void DBusFileManager1::closeTask()
{
    // 如果正在有保险箱的移动、粘贴、删除操作，强行结束任务
    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if (pTaskDlg && pTaskDlg->haveNotCompletedVaultTask()) {
        pTaskDlg->stopVaultTask();
    }
}

//! 写json文件函数
void ChangeJson(const QString & path, const QString & gourpNamne, const QString & VauleName, const QString & vaule)
{
    QByteArray byte;
    QFile file(path);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        byte = file.readAll();
        file.close();
    }else{
        qDebug() <<"openFileError";
        file.close();
        return;
    }

    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byte, &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        qDebug() << " json error ";
    }

    QJsonObject rootobj = jsonDoc.object();
    if (rootobj.contains(gourpNamne)) {
        QJsonObject gourpObject = rootobj.value(gourpNamne).toObject();
        gourpObject[VauleName] = vaule;
        rootobj[gourpNamne] = gourpObject;

        if (file.exists()) {
            file.open(QIODevice::WriteOnly|QIODevice::Text);
            jsonDoc.setObject(rootobj);
            file.seek(0);
            file.write(jsonDoc.toJson());
            file.flush();
            file.close();
        }
    }
}

void DBusFileManager1::lockPropertyChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != "com.deepin.SessionManager")
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "Locked") {
            bool bLocked = changedProps[prop].toBool();
            if (bLocked) {
                char *loginUser = getlogin();
                QString user = loginUser ? loginUser : "";
                emit lockEventTriggered(user);

                char buf[512] = {0};
                FILE *cmd_pipe = popen("pidof -s dde-file-manager", "r");

                fgets(buf, 512, cmd_pipe);
                pid_t pid = static_cast<pid_t>(strtoul(buf, nullptr, 10));

                if (pid == 0) {
                    QString umountCmd = "fusermount -zu " + VAULT_BASE_PATH + "/" + VAULT_DECRYPT_DIR_NAME;
                    QByteArray umountCmdBytes = umountCmd.toUtf8();
                    system(umountCmdBytes.data());
                    //! 记录保险箱上锁时间
                    ChangeJson(VAULT_TIME_CONFIG_FILE,
                               QString("VaultTime"),
                               QString("LockTime"),
                               QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                }

                pclose(cmd_pipe);
            }
        }
    }
}
