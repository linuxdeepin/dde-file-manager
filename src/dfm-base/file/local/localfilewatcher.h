// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEWATCHER_H
#define LOCALFILEWATCHER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <dfm-io/dfileinfo.h>

#include <QObject>

class QUrl;
namespace dfmbase {
class LocalFileWatcher : public AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit LocalFileWatcher() = delete;
    explicit LocalFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~LocalFileWatcher() override;

    virtual void notifyFileAdded(const QUrl &url) override;
    virtual void notifyFileChanged(const QUrl &url) override;
    virtual void notifyFileDeleted(const QUrl &url) override;

    //debug function
    static QStringList getMonitorFiles();
};
}

#endif   // LOCALFILEWATCHER_H
