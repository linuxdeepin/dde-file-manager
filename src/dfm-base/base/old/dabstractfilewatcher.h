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

class QUrl;
class DAbstractFileWatcherPrivate;
class DAbstractFileWatcher : public QObject
{
    Q_OBJECT

public:
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

Q_SIGNALS:
    void fileDeleted(const QUrl &url);
    void fileAttributeChanged(const QUrl &url, const int &isExternalSource = 1);
    void fileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void subfileCreated(const QUrl &url);
    void fileModified(const QUrl &url);
    void fileClosed(const QUrl &url);

protected:
    explicit DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd, const QUrl &url, QObject *parent = nullptr);

    QScopedPointer<DAbstractFileWatcherPrivate> d_ptr;

private:
    Q_DISABLE_COPY(DAbstractFileWatcher)
    Q_DECLARE_PRIVATE(DAbstractFileWatcher)
};

#endif // DABSTRACTFILEWATCHER_H
