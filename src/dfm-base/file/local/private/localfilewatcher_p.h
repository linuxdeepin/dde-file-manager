// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEWATCHER_P_H
#define LOCALFILEWATCHER_P_H

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>
#include <dfm-base/utils/threadcontainer.h>

#include <dfm-io/dwatcher.h>

#include <QUrl>

USING_IO_NAMESPACE
namespace dfmbase {
class LocalFileWatcherPrivate : public AbstractFileWatcherPrivate
{
    Q_OBJECT
    friend class LocalFileWatcher;

public:
    explicit LocalFileWatcherPrivate(const QUrl &fileUrl, LocalFileWatcher *qq);
    virtual ~LocalFileWatcherPrivate() {}
    virtual bool start();
    virtual bool stop();
    void initFileWatcher();
    void initConnect();

private:
    QSharedPointer<DWatcher> watcher { nullptr };   // dfm-io的文件监视器
};
}

#endif   // LOCALFILEWATCHER_P_H
