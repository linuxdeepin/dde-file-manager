/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef ABSTRACTFILEWATCHER_P_H
#define ABSTRACTFILEWATCHER_P_H

#include "base/abstractfilewatcher.h"
#include "utils/threadcontainer.hpp"

#include <dfm-io/core/dwatcher.h>

#include <QUrl>

USING_IO_NAMESPACE
DFMBASE_BEGIN_NAMESPACE
class AbstractFileWatcherPrivate
{
public:
    explicit AbstractFileWatcherPrivate(AbstractFileWatcher *qq);
    virtual ~AbstractFileWatcherPrivate() {}
    virtual bool start();
    virtual bool stop();

    static QString formatPath(const QString &path);

    AbstractFileWatcher *q_ptr;

    QAtomicInteger<bool> started = false;             // 是否开始监视
    QUrl url;                                         // 监视文件的url
    QString path;                                     // 监视文件的路径
    QSharedPointer<DWatcher> watcher = nullptr;       // dfm-io的文件监视器
    static DThreadList<QString> WatcherPath;          // 全局监视文件的监视列表

    Q_DECLARE_PUBLIC(AbstractFileWatcher)
};
DFMBASE_END_NAMESPACE

#endif // DABSTRACTFILEWATCHER_P_H
