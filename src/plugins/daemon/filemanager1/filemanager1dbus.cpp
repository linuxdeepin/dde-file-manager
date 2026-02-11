// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"   //cmake
#include "daemonplugin_filemanager1_global.h"
#include "filemanager1dbus.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-framework/dpf.h>

#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>

DAEMONPFILEMANAGER1_USE_NAMESPACE

FileManager1DBus::FileManager1DBus(QObject *parent)
    : QObject(parent)
{
}

/*!
 * \brief ShowFolders assumes that the specified URIs are folders;
 * the file manager is supposed to show a window with the contents of each folder.
 * Calling this method with file:///etc as the single element in the array of URIs will cause the file manager to
 * show the contents of /etc as if the user had navigated to it. The behavior for more than one element is left up to
 * the implementation; commonly, multiple windows will be shown, one for each folder.
 */
void FileManager1DBus::ShowFolders(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)
    fmInfo() << "[FileManager1DBus] ShowFolders request - URIs:" << URIs;

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--raw" << URIs)) {
        fmDebug() << "[FileManager1DBus] ShowFolders launched via file-manager.sh";
        return;
    }

    if (QProcess::startDetached(DFM_FILE_MANAGER_BINARY, QStringList() << "--raw" << URIs)) {
        fmDebug() << "[FileManager1DBus] ShowFolders launched via dde-file-manager binary";
    } else {
        fmWarning() << "[FileManager1DBus] Failed to launch file manager for ShowFolders request";
    }
}

/*!
 * \brief ShowItemProperties should cause the file manager to show a "properties" window for the specified URIs.
 * For local Unix files, these properties can be the file permissions, icon used for the files, and other metadata.
 */
void FileManager1DBus::ShowItemProperties(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)
    fmInfo() << "[FileManager1DBus] ShowItemProperties request - URIs:" << URIs;

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--raw"
                                                                 << "-p" << URIs)) {
        fmDebug() << "[FileManager1DBus] ShowItemProperties launched via file-manager.sh";
        return;
    }

    if (QProcess::startDetached(DFM_FILE_MANAGER_BINARY, QStringList() << "--raw"
                                                                       << "-p" << URIs)) {
        fmDebug() << "[FileManager1DBus] ShowItemProperties launched via dde-file-manager binary";
    } else {
        fmWarning() << "[FileManager1DBus] Failed to launch file manager for ShowItemProperties request";
    }
}

/*!
 * \brief ShowItems doesn't make any assumptions as to the type of the URIs.
 * The file manager is supposed to select the passed items within their respective parent folders.
 * Calling this method on file:///etc as the single element in the array of URIs will cause the file manager to
 * show a file listing for "/", with "etc" highlighted. The behavior for more than one element is left up to
 * the implementation.
 */
void FileManager1DBus::ShowItems(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)
    fmInfo() << "[FileManager1DBus] ShowItems request - URIs:" << URIs;

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--show-item" << URIs << "--raw")) {
        fmDebug() << "[FileManager1DBus] ShowItems launched via file-manager.sh";
        return;
    }

    if (QProcess::startDetached(DFM_FILE_MANAGER_BINARY, QStringList() << "--show-item" << URIs << "--raw")) {
        fmDebug() << "[FileManager1DBus] ShowItems launched via dde-file-manager binary";
    } else {
        fmWarning() << "[FileManager1DBus] Failed to launch file manager for ShowItems request";
    }
}

void FileManager1DBus::Trash(const QStringList &URIs)
{
    fmInfo() << "[FileManager1DBus] Trash request - URIs count:" << URIs.size();

    QJsonArray srcArray = QJsonArray::fromStringList(URIs);

    QJsonObject paramObj;
    paramObj.insert("sources", srcArray);

    QJsonObject argsObj;
    argsObj.insert("action", "trash");
    argsObj.insert("params", paramObj);

    QJsonDocument doc(argsObj);
    if (QProcess::startDetached("file-manager.sh",
                                QStringList() << "--event"
                                              << doc.toJson())) {
        fmDebug() << "[FileManager1DBus] Trash operation launched via file-manager.sh";
        return;
    }

    if (QProcess::startDetached(DFM_FILE_MANAGER_BINARY,
                                QStringList() << "--event"
                                              << doc.toJson())) {
        fmDebug() << "[FileManager1DBus] Trash operation launched via dde-file-manager binary";
    } else {
        fmWarning() << "[FileManager1DBus] Failed to launch file manager for Trash request";
    }
}

void FileManager1DBus::Open(const QStringList &URIs)
{
    fmInfo() << "[FileManager1DBus] Open request - URIs count:" << URIs.size();

    QStringList processedArgs;

    // 处理 Base64 编码的参数
    for (const QString &arg : URIs) {
        if (arg.startsWith("B64:")) {
            // 解码 Base64 参数
            QString encodedPart = arg.mid(4);   // 移除 "B64:" 前缀
            QByteArray decoded = QByteArray::fromBase64(encodedPart.toUtf8());
            QString decodedArg = QString::fromUtf8(decoded);
            processedArgs.append(decodedArg);
            fmDebug() << "[FileManager1DBus] Decoded Base64 argument:" << decodedArg;
        } else {
            // 普通参数直接使用
            processedArgs.append(arg);
        }
    }

    if (QProcess::startDetached("file-manager.sh", processedArgs)) {
        fmDebug() << "[FileManager1DBus] Open operation launched via file-manager.sh";
        return;
    }

    if (QProcess::startDetached(DFM_FILE_MANAGER_BINARY, processedArgs)) {
        fmDebug() << "[FileManager1DBus] Open operation launched via dde-file-manager binary";
    } else {
        fmWarning() << "[FileManager1DBus] Failed to launch file manager for Open request";
    }
}
