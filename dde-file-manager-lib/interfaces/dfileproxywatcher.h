/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILEPROXYWATCHER_H
#define DFILEPROXYWATCHER_H

#include "dabstractfilewatcher.h"

#include <functional>

class DFileProxyWatcherPrivate;
class DFileProxyWatcher : public DAbstractFileWatcher
{
public:
    explicit DFileProxyWatcher(const DUrl &url, DAbstractFileWatcher *proxy,
                               std::function<DUrl (const DUrl&)> urlConvertFun,
                               QObject *parent = 0);

private slots:
    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void onSubfileCreated(const DUrl &url);

private:
    Q_DECLARE_PRIVATE(DFileProxyWatcher)
};

#endif // DFILEPROXYWATCHER_H
