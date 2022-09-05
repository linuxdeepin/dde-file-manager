// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILESYSTEMWATCHER_P_H
#define DFILESYSTEMWATCHER_P_H

#include "dfilesystemwatcher.h"

#include <QSocketNotifier>
#include <QHash>
#include <QMap>

class DFileSystemWatcherPrivate
{
    Q_DECLARE_PUBLIC(DFileSystemWatcher)

public:
    DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq);
    ~DFileSystemWatcherPrivate();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    DFileSystemWatcher *q_ptr;

    QStringList files, directories;
    int inotifyFd;
    QHash<QString, int> pathToID;
    QMultiHash<int, QString> idToPath;
    QSocketNotifier notifier;

    // private slots
    void _q_readFromInotify();

private:
    void onFileChanged(const QString &path, bool removed);
    void onDirectoryChanged(const QString &path, bool removed);
};

#endif // DFILESYSTEMWATCHER_P_H
