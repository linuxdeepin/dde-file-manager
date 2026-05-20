// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFYFILESYSTEMWATCHER_P_H
#define INOTIFYFILESYSTEMWATCHER_P_H

#include "inotifyfilesystemwatcher.h"

#include <QSocketNotifier>
#include <QHash>
#include <QMultiHash>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class InotifyFileSystemWatcherPrivate
{
    Q_DECLARE_PUBLIC(InotifyFileSystemWatcher)

public:
    InotifyFileSystemWatcherPrivate(int fd, InotifyFileSystemWatcher *qq);
    ~InotifyFileSystemWatcherPrivate();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    void readFromInotify();

private:
    void onFileChanged(const QString &path, bool removed);
    void onDirectoryChanged(const QString &path, bool removed);

    InotifyFileSystemWatcher *q_ptr;

    QStringList files;
    QStringList directories;
    int inotifyFd { -1 };
    QHash<QString, int> pathToID;
    QMultiHash<int, QString> idToPath;
    QSocketNotifier notifier;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INOTIFYFILESYSTEMWATCHER_P_H
