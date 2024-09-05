// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filemanager1dbus.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-framework/dpf.h>

#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>

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

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--raw" << URIs))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--raw" << URIs);
}

/*!
 * \brief ShowItemProperties should cause the file manager to show a "properties" window for the specified URIs.
 * For local Unix files, these properties can be the file permissions, icon used for the files, and other metadata.
 */
void FileManager1DBus::ShowItemProperties(const QStringList &URIs, const QString &StartupId)
{
    Q_UNUSED(StartupId)

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--raw"
                                                                 << "-p" << URIs))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--raw"
                                                              << "-p" << URIs);
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

    if (QProcess::startDetached("file-manager.sh", QStringList() << "--show-item" << URIs << "--raw"))
        return;

    QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << URIs << "--raw");
}

void FileManager1DBus::Trash(const QStringList &URIs)
{
    QJsonArray srcArray = QJsonArray::fromStringList(URIs);

    QJsonObject paramObj;
    paramObj.insert("sources", srcArray);

    QJsonObject argsObj;
    argsObj.insert("action", "trash");
    argsObj.insert("params", paramObj);

    QJsonDocument doc(argsObj);
    QProcess::startDetached("dde-file-manager", QStringList() << "--event" << doc.toJson());
}
