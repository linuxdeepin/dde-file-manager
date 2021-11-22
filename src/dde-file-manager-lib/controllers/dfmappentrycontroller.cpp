/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "interfaces/dabstractfileinfo.h"
#include "models/dfmappentryfileinfo.h"
#include "dfmappentrycontroller.h"
#include "dfmevent.h"
#include "dfmstandardpaths.h"
#include "private/dabstractfilewatcher_p.h"
#include "dfileproxywatcher.h"
#include "dfilewatcher.h"
#include "app/define.h"

#include <QProcess>

const QList<DAbstractFileInfoPointer> DFMAppEntryController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    Q_UNUSED(event);

    static const QString appEntryPath = DFMStandardPaths::location(DFMStandardPaths::ExtensionsAppEntryPath);
    QDir appEntryDir(appEntryPath);
    if (!appEntryDir.exists())
        return {};

    QList<DAbstractFileInfoPointer> ret;
    auto entries = appEntryDir.entryList(QDir::Files);
    QList<QString> cmds; // for de-duplication
    for (auto entry: entries) {
        if (entry.endsWith(".desktop"))
            entry.remove(QRegularExpression(".desktop$"));
        // make the url to `appentry:///helloworld` to support a convinience way to open in search bar.
        DUrl appEntryUrl(APPENTRY_ROOT + entry);
        DAbstractFileInfoPointer fp(new DFMAppEntryFileInfo(appEntryUrl));
        if (!fp->exists()) {
            qInfo() << "the appentry is in extension folder but not exist: " << fp->fileUrl();
            continue;
        }
        auto entryInfo = dynamic_cast<DFMAppEntryFileInfo *>(fp.data());
        if (!entryInfo)
            continue;
        if (cmds.contains(entryInfo->cmd()))
            continue;
        cmds.append(entryInfo->cmd());
        ret << fp;
    }
    return ret;
}

const DAbstractFileInfoPointer DFMAppEntryController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new DFMAppEntryFileInfo(event->url()));
}

DAbstractFileWatcher *DFMAppEntryController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    Q_UNUSED(event);
    auto entryPath = DFMStandardPaths::location(DFMStandardPaths::ExtensionsAppEntryPath);
    DUrl entryUrl(entryPath);
    auto watcher = new DFileProxyWatcher(entryUrl, new DFileWatcher(entryPath), DFMAppEntryController::localToAppEntry);
    return watcher;
}

bool DFMAppEntryController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    auto e = dMakeEventPointer<DFMCreateFileInfoEvent>(event->sender(), event->url());
    auto info = createFileInfo(e);
    auto appEntryInfo = dynamic_cast<DFMAppEntryFileInfo *>(info.data());
    if (appEntryInfo) {
        auto cmd = appEntryInfo->cmd();
        auto ret = QProcess::startDetached(cmd);
        if (!ret) {
            qWarning() << "QProcess::startDetached(" << cmd << ") failed!";
        }
        return ret;
    }
    qWarning() << "Cannot get the appEntryInfo!";
    return false;
}

DUrl DFMAppEntryController::localToAppEntry(const DUrl &local)
{
    auto path = local.path();
    static const auto entryPath = DFMStandardPaths::location(DFMStandardPaths::ExtensionsAppEntryPath) + "/";
    if (!path.startsWith(entryPath)) {
        qWarning() << "Not extension path: " << entryPath;
        return DUrl();
    }
    if (!path.endsWith(".desktop")) {
        qWarning() << "Not entry file: " << entryPath;
        return DUrl();
    }
    QFileInfo entryFile(path);
    if (entryFile.isDir()) {
        qWarning() << "Not entry file: " << entryPath;
        return DUrl();
    }

    path.remove(entryPath).remove(QRegularExpression(".desktop$"));
    qDebug() << "converted path is: " << path;
    return DUrl(APPENTRY_ROOT + path);
}
