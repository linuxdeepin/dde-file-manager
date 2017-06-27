/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dfilewatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include "dfileservices.h"
#include "dfilesystemwatcher.h"

#include "private/dfilesystemwatcher_p.h"

#include "app/define.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"

#include <QDir>
#include <QDebug>

static QString joinFilePath(const QString &path, const QString &name)
{
    if (path.endsWith(QDir::separator()))
        return path + name;

    return path + QDir::separator() + name;
}

class DFileWatcherPrivate : DAbstractFileWatcherPrivate
{
public:
    DFileWatcherPrivate(DFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() Q_DECL_OVERRIDE;
    bool stop() Q_DECL_OVERRIDE;

    void _q_handleFileDeleted(const QString &path, const QString &parentPath);
    void _q_handleFileAttributeChanged(const QString &path, const QString &parentPath);
    void _q_handleFileMoved(const QString &from, const QString &fromParent, const QString &to, const QString &toParent);
    void _q_handleFileCreated(const QString &path, const QString &parentPath);
    void _q_handleFileModified(const QString &path, const QString &parentPath);
    void _q_handleFileClose(const QString &path, const QString &parentPath);

    static QString formatPath(const QString &path);

    QString path;
    QStringList watchFileList;

    static QMap<QString, int> filePathToWatcherCount;

    Q_DECLARE_PUBLIC(DFileWatcher)
};

QMap<QString, int> DFileWatcherPrivate::filePathToWatcherCount;
Q_GLOBAL_STATIC(DFileSystemWatcher, watcher_file_private)

QStringList parentPathList(const QString &path)
{
    QStringList list;
    QDir dir(path);

    list << path;

    while (dir.cdUp()) {
        list << dir.absolutePath();
    }

    return list;
}

bool DFileWatcherPrivate::start()
{
    Q_Q(DFileWatcher);

    started = true;

    foreach (const QString &path, parentPathList(this->path)) {
        if (watchFileList.contains(path))
            continue;

        if (filePathToWatcherCount.value(path, -1) <= 0) {
            if (!watcher_file_private->addPath(path)) {
                qWarning() << Q_FUNC_INFO << "start watch failed, file path =" << path;
                q->stopWatcher();
                started = false;
                return false;
            }
        }

        watchFileList << path;
        filePathToWatcherCount[path] = filePathToWatcherCount.value(path, 0) + 1;
    }

    q->connect(watcher_file_private, &DFileSystemWatcher::fileDeleted,
               q, &DFileWatcher::onFileDeleted);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileAttributeChanged,
               q, &DFileWatcher::onFileAttributeChanged);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileMoved,
               q, &DFileWatcher::onFileMoved);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileCreated,
               q, &DFileWatcher::onFileCreated);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileModified,
               q, &DFileWatcher::onFileModified);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileClosed,
               q, &DFileWatcher::onFileClosed);

    return true;
}

bool DFileWatcherPrivate::stop()
{
    Q_Q(DFileWatcher);

    q->disconnect(watcher_file_private, 0, q, 0);

    bool ok = true;

    foreach (const QString &path, watchFileList) {
        int count = filePathToWatcherCount.value(path, 0);

        --count;

        if (count <= 0) {
            filePathToWatcherCount.remove(path);
            watchFileList.removeOne(path);
            ok = ok && watcher_file_private->removePath(path);
        } else {
            filePathToWatcherCount[path] = count;
        }
    }

    return ok;
}

void DFileWatcherPrivate::_q_handleFileDeleted(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileDeleted(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileAttributeChanged(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileAttributeChanged(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileMoved(const QString &from, const QString &fromParent, const QString &to, const QString &toParent)
{
    Q_Q(DFileWatcher);

    if ((fromParent == this->path && toParent == this->path) || from == this->path) {
        emit q->fileMoved(DUrl::fromLocalFile(from), DUrl::fromLocalFile(to));
    } else if (fromParent == this->path) {
        emit q->fileDeleted(DUrl::fromLocalFile(from));
    } else if (watchFileList.contains(from)) {
        emit q->fileDeleted(url);
    } else if (toParent == this->path) {
        emit q->subfileCreated(DUrl::fromLocalFile(to));
    }
}

void DFileWatcherPrivate::_q_handleFileCreated(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->subfileCreated(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileModified(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileModified(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileClose(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileClosed(DUrl::fromLocalFile(path));
}

QString DFileWatcherPrivate::formatPath(const QString &path)
{
    QString p = QFileInfo(path).absoluteFilePath();

    if (p.endsWith(QDir::separator()))
        p.chop(1);

    return p.isEmpty() ? path : p;
}

DFileWatcher::DFileWatcher(const QString &filePath, QObject *parent)
    : DAbstractFileWatcher(*new DFileWatcherPrivate(this), DUrl::fromLocalFile(filePath), parent)
{
    d_func()->path = DFileWatcherPrivate::formatPath(filePath);
}

void DFileWatcher::onFileDeleted(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileDeleted(path, QString());
    else
        d_func()->_q_handleFileDeleted(joinFilePath(path, name), path);
}

void DFileWatcher::onFileAttributeChanged(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileAttributeChanged(path, QString());
    else
        d_func()->_q_handleFileAttributeChanged(joinFilePath(path, name), path);
}

void DFileWatcher::onFileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname)
{
    QString fromPath, fpPath;
    QString toPath, tpPath;

    if (fname.isEmpty()) {
        fromPath = from;
    } else {
        fromPath = joinFilePath(from, fname);
        fpPath = from;
    }

    if (tname.isEmpty()) {
        toPath = to;
    } else {
        toPath = joinFilePath(to, tname);
        tpPath = to;
    }

    d_func()->_q_handleFileMoved(fromPath, fpPath, toPath, tpPath);
}

void DFileWatcher::onFileCreated(const QString &path, const QString &name)
{
    d_func()->_q_handleFileCreated(joinFilePath(path, name), path);
}

void DFileWatcher::onFileModified(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileModified(path, QString());
    else
        d_func()->_q_handleFileModified(joinFilePath(path, name), path);
}

void DFileWatcher::onFileClosed(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileClose(path, QString());
    else
        d_func()->_q_handleFileClose(joinFilePath(path, name), path);
}

QStringList DFileWatcher::getMonitorFiles()
{
    QStringList list;

    list << watcher_file_private->directories();
    list << watcher_file_private->files();

    list << "---------------------------";

    QMap<QString, int>::const_iterator i = DFileWatcherPrivate::filePathToWatcherCount.constBegin();

    while (i != DFileWatcherPrivate::filePathToWatcherCount.constEnd()) {
        list << QString("%1, %2").arg(i.key()).arg(i.value());
        ++i;
    }

    return list;
}

#include "moc_dfilewatcher.cpp"
