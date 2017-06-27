/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILEWATCHER_H
#define DFILEWATCHER_H

#include "dabstractfilewatcher.h"

class DFileWatcherPrivate;
class DFileWatcher : public DAbstractFileWatcher
{
    Q_OBJECT

public:
    explicit DFileWatcher(const QString &filePath, QObject *parent = 0);

    //debug function
    static QStringList getMonitorFiles();

private slots:
    void onFileDeleted(const QString &path, const QString &name);
    void onFileAttributeChanged(const QString &path, const QString &name);
    void onFileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName);
    void onFileCreated(const QString &path, const QString &name);
    void onFileModified(const QString &path, const QString &name);
    void onFileClosed(const QString &path, const QString &name);

private:
    Q_DECLARE_PRIVATE(DFileWatcher)
};

#endif // DFILEWATCHER_H
