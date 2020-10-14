/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILESYSTEMWATCHER_H
#define DFILESYSTEMWATCHER_H

#include <QObject>

class DFileSystemWatcherPrivate;
class DFileSystemWatcher : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFileSystemWatcher)

public:
    explicit DFileSystemWatcher(QObject *parent = Q_NULLPTR);
    DFileSystemWatcher(const QStringList &paths, QObject *parent = Q_NULLPTR);
    ~DFileSystemWatcher();

    bool addPath(const QString &file);
    QStringList addPaths(const QStringList &paths);
    bool removePath(const QString &path);
    QStringList removePaths(const QStringList &paths);

    QStringList files() const;
    QStringList directories() const;

Q_SIGNALS:
    void fileDeleted(const QString &path, const QString &name, QPrivateSignal);
    void fileAttributeChanged(const QString &path, const QString &name, QPrivateSignal);
    void fileClosed(const QString &path, const QString &name, QPrivateSignal);
    void fileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName, QPrivateSignal);
    void fileCreated(const QString &path, const QString &name, QPrivateSignal);
    void fileModified(const QString &path, const QString &name, QPrivateSignal);

    //! 当挂载的文件系统被卸载时发送卸载卸载信号
    void fileSystemUMount(const QString &path, const QString &name, QPrivateSignal);
private:
    QScopedPointer<DFileSystemWatcherPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_readFromInotify())
};

#endif // DFILESYSTEMWATCHER_H
