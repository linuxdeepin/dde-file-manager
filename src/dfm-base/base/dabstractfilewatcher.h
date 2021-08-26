/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DABSTRACTFILEWATCHER_H
#define DABSTRACTFILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>

class QUrl;
class DAbstractFileWatcherPrivate;
class DAbstractFileWatcher : public QObject
{
    Q_OBJECT

public:

    explicit DAbstractFileWatcher(const QUrl &url, QObject *parent = nullptr);

    virtual ~DAbstractFileWatcher();

    QUrl url() const;

    bool startWatcher();
    bool stopWatcher();
    bool restartWatcher();

    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true);

    using SignalType1 = void(DAbstractFileWatcher::*)(const QUrl &);
    using SignalType2 = void(DAbstractFileWatcher::*)(const QUrl &, const QUrl &);
    using SignalType3 = void(DAbstractFileWatcher::*)(const QUrl &, const int &);
    static bool ghostSignal(const QUrl &targetUrl, SignalType1 signal, const QUrl &arg1);
    static bool ghostSignal(const QUrl &targetUrl, SignalType2 signal, const QUrl &arg1, const QUrl &arg2);
    static bool ghostSignal(const QUrl &targetUrl, SignalType3 signal, const QUrl &arg1, const int isExternalSource = 1);

    //debug function
    static QStringList getMonitorFiles();

Q_SIGNALS:
    void fileDeleted(const QUrl &url);
    void fileAttributeChanged(const QUrl &url, const int &isExternalSource = 1);
    void fileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void subfileCreated(const QUrl &url);
    void fileModified(const QUrl &url);
    void fileClosed(const QUrl &url);

private Q_SLOTS:
    void onFileDeleted(const QString &path, const QString &name);
    void onFileAttributeChanged(const QString &path, const QString &name);
    void onFileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname);
    void onFileCreated(const QString &path, const QString &name);
    void onFileModified(const QString &path, const QString &name);
    void onFileClosed(const QString &path, const QString &name);

    //! 处理文件系统卸载事件如U盘、Cryfs加密保险箱
    void onFileSystemUMount(const QString &path, const QString &name);

protected:
    explicit DAbstractFileWatcher(QObject* parent = nullptr);
    explicit DAbstractFileWatcher(const QString &filePath, QObject *parent = nullptr);
    explicit DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd, const QUrl &url, QObject *parent = nullptr);
    QScopedPointer<DAbstractFileWatcherPrivate> d_ptr;

private:
    Q_DISABLE_COPY(DAbstractFileWatcher)
    Q_DECLARE_PRIVATE(DAbstractFileWatcher)
};

typedef QSharedPointer<DAbstractFileWatcher> DAbstractFileWatcherPointer;

#endif // DABSTRACTFILEWATCHER_H
