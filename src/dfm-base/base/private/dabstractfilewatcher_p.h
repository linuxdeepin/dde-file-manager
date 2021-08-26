/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DABSTRACTFILEWATCHER_P_H
#define DABSTRACTFILEWATCHER_P_H

#include <QUrl>
#include "base/dabstractfilewatcher.h"

class DAbstractFileWatcherPrivate
{
public:
    explicit DAbstractFileWatcherPrivate(DAbstractFileWatcher *qq);
    virtual ~DAbstractFileWatcherPrivate() {}
    virtual bool start();
    virtual bool stop();
    virtual bool handleGhostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const QUrl &arg1);
    virtual bool handleGhostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const QUrl &arg1, const QUrl &arg2);
    virtual bool handleGhostSignal(const QUrl &targetUrl, DAbstractFileWatcher::SignalType3 signal, const QUrl &arg1, int isExternalSource);

    void _q_handleFileDeleted(const QString &path, const QString &parentPath);
    void _q_handleFileAttributeChanged(const QString &path, const QString &parentPath);
    bool _q_handleFileMoved(const QString &from, const QString &fromParent, const QString &to, const QString &toParent);
    void _q_handleFileCreated(const QString &path, const QString &parentPath);
    void _q_handleFileModified(const QString &path, const QString &parentPath);
    void _q_handleFileClose(const QString &path, const QString &parentPath);

    static QString formatPath(const QString &path);

    QString path;
    QStringList watchFileList;

    static QMap<QString, int> filePathToWatcherCount;

    DAbstractFileWatcher *q_ptr;

    QUrl url;
    bool started = false;
    static QList<DAbstractFileWatcher *> watcherList;

    Q_DECLARE_PUBLIC(DAbstractFileWatcher)
};

#endif // DABSTRACTFILEWATCHER_P_H
