// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTFILEWATCHER_P_H
#define ABSTRACTFILEWATCHER_P_H

#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/utils/threadcontainer.h>

#include <dfm-io/dwatcher.h>

#include <QUrl>

USING_IO_NAMESPACE
namespace dfmbase {
class AbstractFileWatcherPrivate : public QObject
{
    Q_DISABLE_COPY(AbstractFileWatcherPrivate)
    friend class AbstractFileWatcher;

public:
    explicit AbstractFileWatcherPrivate(const QUrl &fileUrl, AbstractFileWatcher *qq);
    virtual ~AbstractFileWatcherPrivate() {}
    virtual bool start();
    virtual bool stop();
    static QString formatPath(const QString &path);

protected:
    AbstractFileWatcher *q = nullptr;
    QAtomicInteger<bool> started { false };   // 是否开始监视
    QAtomicInt cacheInfoConnectSize { 0 };   // Cached FileInfo, number of connections monitored
    QUrl url;   // 监视文件的url
    QString path;   // 监视文件的路径
};
}

#endif   // DABSTRACTFILEWATCHER_P_H
