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
#include "dabstractfileinfo.h"
#include "dfilesystemwatcher.h"

#include "app/define.h"
#include "widgets/singleton.h"
#include "usershare/usersharemanager.h"

#include <QDir>
#include <QDebug>

class DFileWatcherPrivate : DAbstractFileWatcherPrivate
{
public:
    DFileWatcherPrivate(DFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    void _q_handleFileDeleted(const DUrl &url, const DUrl &parentUrl);
    void _q_handleFileAttributeChanged(const DUrl &url, const DUrl &parentUrl);
    void _q_handleFileMoved(const DUrl &from, const DUrl &fromParent, const DUrl &to, const DUrl &toParent);
    void _q_handleFileCreated(const DUrl &url, const DUrl &parentUrl);
    void _q_onUserShareInfoChanged(const QString &path);

    DUrlList watchUrlList;

    static QMap<DUrl, int> urlToWatcherCount;

    Q_DECLARE_PUBLIC(DFileWatcher)
};

QMap<DUrl, int> DFileWatcherPrivate::urlToWatcherCount;

void DFileWatcherPrivate::_q_handleFileDeleted(const DUrl &url, const DUrl &parentUrl)
{
    if (url != this->url && parentUrl != this->url)
        return;

    Q_Q(DFileWatcher);

    emit q->fileDeleted(url);
}

void DFileWatcherPrivate::_q_handleFileAttributeChanged(const DUrl &url, const DUrl &parentUrl)
{
    if (url != this->url && parentUrl != this->url)
        return;

    Q_Q(DFileWatcher);

    emit q->fileAttributeChanged(url);
}

void DFileWatcherPrivate::_q_handleFileMoved(const DUrl &from, const DUrl &fromParent, const DUrl &to, const DUrl &toParent)
{
    Q_Q(DFileWatcher);

    if ((fromParent == this->url && toParent == this->url) || from == this->url) {
        emit q->fileMoved(from, to);
    } else if (fromParent == this->url) {
        emit q->fileDeleted(from);
    } else if (watchUrlList.contains(fromParent)) {
        emit q->fileDeleted(this->url);
    } else if (toParent == this->url) {
        emit q->subfileCreated(to);
    }
}

void DFileWatcherPrivate::_q_handleFileCreated(const DUrl &url, const DUrl &parentUrl)
{
    if (parentUrl != this->url)
        return;

    Q_Q(DFileWatcher);

    emit q->subfileCreated(url);
}

void DFileWatcherPrivate::_q_onUserShareInfoChanged(const QString &path)
{
    QFileInfo info(path);

    if (path == url.toLocalFile()
            || info.absolutePath() == url.toLocalFile()) {
        Q_Q(DFileWatcher);

        emit q->fileAttributeChanged(url);
    }
}

Q_GLOBAL_STATIC(DFileSystemWatcher, watcher_file_private)

DFileWatcher::DFileWatcher(const QString &filePath, QObject *parent)
    : DAbstractFileWatcher(*new DFileWatcherPrivate(this), DUrl::fromLocalFile(filePath), parent)
{
    connect(userShareManager, SIGNAL(userShareAdded(QString)), this, SLOT(_q_onUserShareInfoChanged(QString)));
    connect(userShareManager, SIGNAL(userShareDeleted(QString)), this, SLOT(_q_onUserShareInfoChanged(QString)));
}

DFileWatcher::~DFileWatcher()
{
    d_func()->started = false;
    DFileWatcher::stop();
}

void DFileWatcher::onFileDeleted(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileDeleted(DUrl::fromLocalFile(path), DUrl());
    else
        d_func()->_q_handleFileDeleted(DUrl::fromLocalFile(path + QDir::separator() + name),
                                       DUrl::fromLocalFile(path));
}

void DFileWatcher::onFileAttributeChanged(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileAttributeChanged(DUrl::fromLocalFile(path), DUrl());
    else
        d_func()->_q_handleFileAttributeChanged(DUrl::fromLocalFile(path + QDir::separator() + name),
                                                DUrl::fromLocalFile(path));
}

void DFileWatcher::onFileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname)
{
    DUrl furl, fpurl;
    DUrl turl, tpurl;

    if (fname.isEmpty()) {
        furl = DUrl::fromLocalFile(from);
    } else {
        furl = DUrl::fromLocalFile(from + QDir::separator() + fname);
        fpurl = DUrl::fromLocalFile(from);
    }

    if (tname.isEmpty()) {
        turl = DUrl::fromLocalFile(to);
    } else {
        turl = DUrl::fromLocalFile(to + QDir::separator() + tname);
        tpurl = DUrl::fromLocalFile(to);
    }

    d_func()->_q_handleFileMoved(furl, fpurl, turl, tpurl);
}

void DFileWatcher::onFileCreated(const QString &path, const QString &name)
{
    d_func()->_q_handleFileCreated(DUrl::fromLocalFile(path + QDir::separator() + name), DUrl::fromLocalFile(path));
}

bool DFileWatcher::start()
{
    Q_D(DFileWatcher);

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(d->url);

    if (!fileInfo)
        return false;

    d->started = true;

    DUrlList parentList = fileInfo->parentUrlList();

    parentList.prepend(d->url);

    foreach (const DUrl &url, parentList) {
        if (d->watchUrlList.contains(url))
            continue;

        if (d->urlToWatcherCount.value(url, -1) <= 0) {
            if (!watcher_file_private->addPath(url.toLocalFile())) {
                qWarning() << Q_FUNC_INFO << "start watch failed, url =" << d->url;
                stopWatcher();
                d->started = false;
                return false;
            }
        }

        d->watchUrlList << url;
        d->urlToWatcherCount[url] = d->urlToWatcherCount.value(url, 0) + 1;
    }

    connect(watcher_file_private, &DFileSystemWatcher::fileDeleted,
            this, &DFileWatcher::onFileDeleted);
    connect(watcher_file_private, &DFileSystemWatcher::fileAttributeChanged,
            this, &DFileWatcher::onFileAttributeChanged);
    connect(watcher_file_private, &DFileSystemWatcher::fileMoved,
            this, &DFileWatcher::onFileMoved);
    connect(watcher_file_private, &DFileSystemWatcher::fileCreated,
            this, &DFileWatcher::onFileCreated);

    return true;
}

bool DFileWatcher::stop()
{
    Q_D(DFileWatcher);

    disconnect(watcher_file_private, 0, this, 0);

    bool ok = true;

    foreach (const DUrl &url, d->watchUrlList) {
        int count = d->urlToWatcherCount.value(url, 0);

        --count;

        if (count <= 0) {
            d->urlToWatcherCount.remove(url);
            d->watchUrlList.removeOne(url);
            ok = ok && watcher_file_private->removePath(url.toLocalFile());
        } else {
            d->urlToWatcherCount[url] = count;
        }
    }

    return ok;
}

#include "moc_dfilewatcher.cpp"
