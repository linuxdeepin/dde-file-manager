/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "base/dabstractfilewatcher.h"
#include "base/private/dabstractfilewatcher_p.h"
#include "base/private/dfilesystemwatcher.h"
#include "base/singleton.hpp"
#include "base/dfmurlroute.h"

#include <QEvent>
#include <QDir>
#include <QDebug>

static QString joinFilePath(const QString &path, const QString &name)
{
    if (path.endsWith(QDir::separator()))
        return path + name;

    return path + QDir::separator() + name;
}

bool isPathWatched(const QString &path);

QStringList parentPathList(const QString &path)
{
    QStringList list;
    QDir dir(path);

    list << path;

    QString strTmpPath = path;
    // fix bug#27870 往已刻录的文件夹中的文件夹...中添加文件夹，界面不刷新
    // 往已刻录的文件夹中放置文件时，由于父路径可能不存在，导致不能创建对应的 watcher ，因此不能监听到文件夹变化，导致界面不刷新
    if (!dir.exists() && path.contains("/.cache/deepin/discburn/")) // 目前仅针对光驱刻录的暂存区进行处理
        dir.mkdir(path);

    while (dir.cdUp()) {
        //! fixed the bug that directory not refresh when same pathes in the list.
        if (isPathWatched(dir.absolutePath())) {
            continue;
        }
        list << dir.absolutePath();
    }

    return list;
}

QMap<QString, int> DAbstractFileWatcherPrivate::filePathToWatcherCount;
Q_GLOBAL_STATIC(DFileSystemWatcher, watcher_file_private)
QList<DAbstractFileWatcher *> DAbstractFileWatcherPrivate::watcherList;
DAbstractFileWatcherPrivate::DAbstractFileWatcherPrivate(DAbstractFileWatcher *qq)
    : q_ptr(qq)
{

}

bool isPathWatched(const QString &path)
{
    QFileInfo fi(path);
    return (fi.isDir() && watcher_file_private->directories().contains(path)) ||
           (fi.isFile() && watcher_file_private->files().contains(path));
}


bool DAbstractFileWatcherPrivate::start()
{
    Q_Q(DAbstractFileWatcher);

    started = true;

    Q_FOREACH (const QString &path, parentPathList(this->path)) {
        if (watchFileList.contains(path))
            continue;

        if (filePathToWatcherCount.value(path, -1) <= 0 || !isPathWatched(path)) {
            if (QFile::exists(path)) {
                bool shouldAddToPath = true;
                if (shouldAddToPath && !watcher_file_private->addPath(path)) {
                    qWarning() << Q_FUNC_INFO << "start watch failed, file path =" << path;
                    q->stopWatcher();
                    started = false;
                    return false;
                }
            }
        }

        watchFileList << path;
        filePathToWatcherCount[path] = filePathToWatcherCount.value(path, 0) + 1;
    }

    q->connect(watcher_file_private, &DFileSystemWatcher::fileDeleted,
               q, &DAbstractFileWatcher::onFileDeleted);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileAttributeChanged,
               q, &DAbstractFileWatcher::onFileAttributeChanged);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileMoved,
               q, &DAbstractFileWatcher::onFileMoved);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileCreated,
               q, &DAbstractFileWatcher::onFileCreated);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileModified,
               q, &DAbstractFileWatcher::onFileModified);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileClosed,
               q, &DAbstractFileWatcher::onFileClosed);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileSystemUMount,
               q, &DAbstractFileWatcher::onFileSystemUMount);

//    q->connect(fileSignalManager, &FileSignalManager::fileMoved,
//               q, &DAbstractFileWatcher::onFileMoved);
    return true;
}

bool DAbstractFileWatcherPrivate::stop()
{
    Q_Q(DAbstractFileWatcher);

    if (watcher_file_private.isDestroyed())
        return true;

//    q->disconnect(watcher_file_private, 0, q, 0);//避免0值警告
    q->disconnect(watcher_file_private, nullptr, q, nullptr);

    bool ok = true;

    for (auto it = watchFileList.begin(); it != watchFileList.end();) {
        int count = filePathToWatcherCount.value(*it, 0);

        --count;
        if (count > 0 && !isPathWatched(*it)) { // already removed from DFileSystemWatcher
            filePathToWatcherCount.remove(*it);
            it = watchFileList.erase(it);
            continue;
        }

        if (count <= 0) {
            filePathToWatcherCount.remove(*it);
            ok = ok && watcher_file_private->removePath(*it);
            it = watchFileList.erase(it);
            continue;
        } else {
            filePathToWatcherCount[*it] = count;
        }

        ++it;
    }

    return ok;
}

bool DAbstractFileWatcherPrivate::handleGhostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const QUrl &arg1)
{
    if (signal == &DAbstractFileWatcher::fileDeleted) {
        for (const QString &path : watchFileList) {
            const QUrl &_url = QUrl::fromLocalFile(path);

            if (_url == arg1) {
                q_ptr->fileDeleted(this->url);

                return true;
            }
        }
    } else {
        if (url == targetUrl || url == arg1) {
            (q_ptr->*signal)(arg1);

            return true;
        }
        return false;
    }

    return false;


}

bool DAbstractFileWatcherPrivate::handleGhostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType3 signal, const QUrl &arg1, int isExternalSource)
{

    if (url == targetUrl || url == arg1) {
        (q_ptr->*signal)(arg1, isExternalSource);

        return true;
    }

    return false;
}

bool DAbstractFileWatcherPrivate::handleGhostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const QUrl &arg1, const QUrl &arg2)
{

    if (!targetUrl.isLocalFile())
        return false;

    if (signal != &DAbstractFileWatcher::fileMoved) {
        if (url == targetUrl || url == arg1 || url == arg2) {
            (q_ptr->*signal)(arg1, arg2);
            return true;
        }

        return false;
    }

    return _q_handleFileMoved(arg1.toLocalFile(), DFMUrlRoute::urlToPath(DFMUrlRoute::urlParent(arg1)),
                              arg2.toLocalFile(), DFMUrlRoute::urlToPath(DFMUrlRoute::urlParent(DFMUrlRoute::urlParent(arg1))));
}

DAbstractFileWatcher::~DAbstractFileWatcher(){
    stopWatcher();
    DAbstractFileWatcherPrivate::watcherList.removeOne(this);
}

void DAbstractFileWatcherPrivate::_q_handleFileDeleted(const QString &path, const QString &parentPath)
{
    Q_Q(DAbstractFileWatcher);

    //如果被删除目录是当前目录的父级及以上目录
    //则需要发出当前目录也被删除的事件
    if (this->path.startsWith(path) && path != this->path) {
        Q_EMIT q->fileDeleted(QUrl::fromLocalFile(this->path));
        return;
    }

    //如果被删除的目录是当前目录或当前目录的子目录
    //则需要发出被删除目录已被删除的事件
    if (path != this->path && parentPath != this->path)
        return;

    Q_EMIT q->fileDeleted(QUrl::fromLocalFile(path));
}

void DAbstractFileWatcherPrivate::_q_handleFileAttributeChanged(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DAbstractFileWatcher);

    Q_EMIT q->fileAttributeChanged(QUrl::fromLocalFile(path));
}

bool DAbstractFileWatcherPrivate::_q_handleFileMoved(const QString &from, const QString &fromParent, const QString &to, const QString &toParent)
{
    Q_Q(DAbstractFileWatcher);

    if ((fromParent == this->path && toParent == this->path) || from == this->path) {
        Q_EMIT q->fileMoved(QUrl::fromLocalFile(from), QUrl::fromLocalFile(to));
    } else if (fromParent == this->path) {
        Q_EMIT q->fileDeleted(QUrl::fromLocalFile(from));
    } else if (watchFileList.contains(from)) {
        Q_EMIT q->fileDeleted(url);
    } else if (toParent == this->path) {
        Q_EMIT q->subfileCreated(QUrl::fromLocalFile(to));
    } else {
        return false;
    }

    return true;
}

void DAbstractFileWatcherPrivate::_q_handleFileCreated(const QString &path, const QString &parentPath)
{
    if (watchFileList.contains(path)) {
        bool result = watcher_file_private->addPath(path);
        if (!result) {
            qWarning() << Q_FUNC_INFO << "add to watcher failed, file path =" << path;
        }
    }

    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DAbstractFileWatcher);

    Q_EMIT q->subfileCreated(QUrl::fromLocalFile(path));
}

void DAbstractFileWatcherPrivate::_q_handleFileModified(const QString &path, const QString &parentPath)
{
    //if (path != this->path && parentPath != this->path)
    // bug 25533: some path add some external path when modified, so use the contain function
    if (!path.contains(this->path) && parentPath != this->path)
        return;

    Q_Q(DAbstractFileWatcher);

    Q_EMIT q->fileModified(QUrl::fromLocalFile(path));
}

void DAbstractFileWatcherPrivate::_q_handleFileClose(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DAbstractFileWatcher);

    Q_EMIT q->fileClosed(QUrl::fromLocalFile(path));
}

QString DAbstractFileWatcherPrivate::formatPath(const QString &path)
{
    QString p = QFileInfo(path).absoluteFilePath();

    if (p.endsWith(QDir::separator()))
        p.chop(1);

    return p.isEmpty() ? path : p;
}

DAbstractFileWatcher::DAbstractFileWatcher(QObject *parent)
    :QObject (parent)
    ,d_ptr(new DAbstractFileWatcherPrivate(this))
{

}

DAbstractFileWatcher::DAbstractFileWatcher(const QString &filePath, QObject *parent)
    : DAbstractFileWatcher(*new DAbstractFileWatcherPrivate(this), QUrl::fromLocalFile(filePath), parent)
{
    d_func()->path = DAbstractFileWatcherPrivate::formatPath(filePath);
}

DAbstractFileWatcher::DAbstractFileWatcher(const QUrl &url, QObject *parent)
    : DAbstractFileWatcher(DFMUrlRoute::urlToPath(url),parent)
{

}

QUrl DAbstractFileWatcher::url() const
{
    Q_D(const DAbstractFileWatcher);

    return d->url;
}

bool DAbstractFileWatcher::startWatcher()
{
    Q_D(DAbstractFileWatcher);

    if (d->started)
        return true;

    if (d->start()) {
        d->started = true;

        return true;
    }

    return false;
}

bool DAbstractFileWatcher::stopWatcher()
{
    Q_D(DAbstractFileWatcher);

    if (!d->started)
        return false;

    if (d->stop()) {
        d->started = false;

        return true;
    }

    return false;
}

bool DAbstractFileWatcher::restartWatcher()
{
    bool ok = stopWatcher();
    return ok && startWatcher();
}

void DAbstractFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    Q_UNUSED(subfileUrl)
    Q_UNUSED(enabled)
}

bool DAbstractFileWatcher::ghostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const QUrl &arg1)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DAbstractFileWatcher *watcher : DAbstractFileWatcherPrivate::watcherList) {
        if (watcher->d_func()->handleGhostSignal(targetUrl, signal, arg1))
            ok = true;
    }

    return ok;
}

bool DAbstractFileWatcher::ghostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType3 signal, const QUrl &arg1, const int isExternalSource)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DAbstractFileWatcher *watcher : DAbstractFileWatcherPrivate::watcherList) {
        if (watcher->d_func()->handleGhostSignal(targetUrl, signal, arg1, isExternalSource))
            ok = true;
    }

    return ok;
}

bool DAbstractFileWatcher::ghostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const QUrl &arg1, const QUrl &arg2)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DAbstractFileWatcher *watcher : DAbstractFileWatcherPrivate::watcherList) {
        if (watcher->d_func()->handleGhostSignal(targetUrl, signal, arg1, arg2))
            ok = true;
    }

    return ok;
}

DAbstractFileWatcher::DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd,
                                           const QUrl &url, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_ASSERT(url.isValid());

//    qDebug() << "add file watcher for: " << url;

    d_ptr->url = url;
    DAbstractFileWatcherPrivate::watcherList << this;
}

void DAbstractFileWatcher::onFileDeleted(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileDeleted(path, QString());
    else
        d_func()->_q_handleFileDeleted(joinFilePath(path, name), path);
}

void DAbstractFileWatcher::onFileAttributeChanged(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileAttributeChanged(path, QString());
    else
        d_func()->_q_handleFileAttributeChanged(joinFilePath(path, name), path);
}

void DAbstractFileWatcher::onFileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname)
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

void DAbstractFileWatcher::onFileCreated(const QString &path, const QString &name)
{
    d_func()->_q_handleFileCreated(joinFilePath(path, name), path);
}

void DAbstractFileWatcher::onFileModified(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileModified(path, QString());
    else
        d_func()->_q_handleFileModified(joinFilePath(path, name), path);
}

void DAbstractFileWatcher::onFileClosed(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileClose(path, QString());
    else
        d_func()->_q_handleFileClose(joinFilePath(path, name), path);
}

void DAbstractFileWatcher::onFileSystemUMount(const QString &path, const QString &name)
{
    Q_UNUSED(name)

    d_func()->filePathToWatcherCount.remove(path);
    watcher_file_private->removePath(path);
    d_func()->watchFileList.removeOne(path);
}

QStringList DAbstractFileWatcher::getMonitorFiles()
{
    QStringList list;

    list << watcher_file_private->directories();
    list << watcher_file_private->files();

    list << "---------------------------";

    QMap<QString, int>::const_iterator i = DAbstractFileWatcherPrivate::filePathToWatcherCount.constBegin();

    while (i != DAbstractFileWatcherPrivate::filePathToWatcherCount.constEnd()) {
        list << QString("%1, %2").arg(i.key()).arg(i.value());
        ++i;
    }

    return list;
}

//#include "moc_dabstractfilewatcher.cpp"
