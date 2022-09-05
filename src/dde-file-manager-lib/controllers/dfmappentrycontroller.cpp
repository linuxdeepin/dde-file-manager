// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        return QList<DAbstractFileInfoPointer>();

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
