/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef FILEMONITORWOKER_H
#define FILEMONITORWOKER_H

#include <QtCore>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#define RichDirPrefix ".deepin_rich_dir_"
#define desktopLocation QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0)

#define EVENT_NUM 16
#define MAX_BUF_SIZE 1024

class FileMonitorWoker : public QObject
{
    Q_OBJECT
public:
    explicit FileMonitorWoker(QObject *parent = 0);
    ~FileMonitorWoker();

    void initInotify();


signals:
    void monitorFolderChanged(const QString& path);
    void fileCreated(int cookie, QString path);
    void fileMovedFrom(int cookie, QString path);
    void fileMovedTo(int cookie, QString out);
    void fileDeleted(int cookie, QString path);
    void metaDataChanged(int cookie, QString path);

    void fileChanged(QString path);
    void directoryChanged(QString path);

public slots:
    inline bool addPath(const QString &path)
    { return addPaths(QStringList() << path);}
    inline bool removePath(const QString &path)
    { return removePaths(QStringList() << path);}

    bool addPaths(const QStringList &list);
    bool removePaths(const QStringList &list);

private slots:
    void readFromInotify();

private:
//    void fileChanged(const QString &path, bool removed);
//    void directoryChanged(const QString &path, bool removed);

    void handleInotifyEvent(const struct inotify_event* event);

    int addWatch(const QString &path, bool isDir);
    int rmWatch(const QString &path);

private:
    QString getPathFromID(int id) const;
    QStringList addPathsAction(const QStringList &paths);
    QStringList removePathsAction(const QStringList &paths);

private:
    int m_inotifyFd;

    QSocketNotifier *m_notifier;
    QHash<QString, int> m_pathToID;
    QMultiHash<int, QString> m_idToPath;
    QMap<QString, int> m_pathReferenceCounts;
};

#endif // FILEMONITORWOKER_H
