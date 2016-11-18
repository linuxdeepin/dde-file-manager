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

class DUrl;
class DAbstractFileWatcherPrivate;
class DAbstractFileWatcher : public QObject
{
    Q_OBJECT

public:
    ~DAbstractFileWatcher();

    DUrl fileUrl() const;

    bool startWatcher();
    bool stopWatcher();
    bool restartWatcher();

    virtual void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true);

signals:
    void fileDeleted(const DUrl &url);
    void fileAttributeChanged(const DUrl &url);
    void fileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void subfileCreated(const DUrl &url);
    void fileModified(const DUrl &url);

protected:
    explicit DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd, const DUrl &url, QObject *parent = 0);

    QScopedPointer<DAbstractFileWatcherPrivate> d_ptr;

private:
    Q_DISABLE_COPY(DAbstractFileWatcher)
    Q_DECLARE_PRIVATE(DAbstractFileWatcher)
};

#endif // DABSTRACTFILEWATCHER_H
