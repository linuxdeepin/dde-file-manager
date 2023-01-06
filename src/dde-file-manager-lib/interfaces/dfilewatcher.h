// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEWATCHER_H
#define DFILEWATCHER_H

#include "dabstractfilewatcher.h"

class DFileWatcherPrivate;
class DFileWatcher : public DAbstractFileWatcher
{
    Q_OBJECT

public:
    explicit DFileWatcher(const QString &filePath, QObject *parent = nullptr);

    //debug function
    static QStringList getMonitorFiles();

private slots:
    void onFileDeleted(const QString &path, const QString &name);
    void onFileAttributeChanged(const QString &path, const QString &name);
    void onFileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname);
    void onFileCreated(const QString &path, const QString &name);
    void onFileModified(const QString &path, const QString &name);
    void onFileClosed(const QString &path, const QString &name);

    //! 处理文件系统卸载事件如U盘、Cryfs加密保险箱
    void onFileSystemUMount(const QString &path, const QString &name);

private:
    Q_DECLARE_PRIVATE(DFileWatcher)
};

#endif // DFILEWATCHER_H
