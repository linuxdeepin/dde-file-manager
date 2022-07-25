/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "filemanager1dbus.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <dfm-framework/dpf.h>

#include <QProcess>

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
    QList<QUrl> urls;
    for (const QString &path : URIs) {
        QUrl tempUrl { DFMBASE_NAMESPACE::UrlRoute::fromUserInput(path) };
        urls << tempUrl;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash,
                                          0, urls, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint, nullptr);
}
