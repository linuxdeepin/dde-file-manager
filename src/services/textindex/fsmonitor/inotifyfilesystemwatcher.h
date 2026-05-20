// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFYFILESYSTEMWATCHER_H
#define INOTIFYFILESYSTEMWATCHER_H

#include "service_textindex_global.h"

#include <QObject>
#include <QStringList>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class InotifyFileSystemWatcherPrivate;

// InotifyFileSystemWatcher: Linux-only file system watcher using inotify
// Adapted from DTK DFileSystemWatcher with directory IN_CLOSE_WRITE support
//
// Key difference from DFileSystemWatcher:
//   - Directory watch mask includes IN_CLOSE_WRITE, so fileClosed signal
//     is emitted when a file within a watched directory finishes writing.
//   - This avoids the high-frequency fileModified (IN_MODIFY) signals during
//     large file copies, improving performance for indexing use cases.
class InotifyFileSystemWatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(InotifyFileSystemWatcher)
    Q_DECLARE_PRIVATE(InotifyFileSystemWatcher)

public:
    explicit InotifyFileSystemWatcher(QObject *parent = nullptr);
    ~InotifyFileSystemWatcher() override;

    bool addPath(const QString &path);
    QStringList addPaths(const QStringList &paths);

    bool removePath(const QString &path);
    QStringList removePaths(const QStringList &paths);

    QStringList files() const;
    QStringList directories() const;

Q_SIGNALS:
    void fileCreated(const QString &path, const QString &name);
    void fileDeleted(const QString &path, const QString &name);
    void fileClosed(const QString &path, const QString &name);
    void fileModified(const QString &path, const QString &name);
    void fileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName);
    void fileAttributeChanged(const QString &path, const QString &name);

private:
    QScopedPointer<InotifyFileSystemWatcherPrivate> d_ptr;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INOTIFYFILESYSTEMWATCHER_H
