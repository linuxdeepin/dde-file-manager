// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTFILEWATCHER_H
#define RECENTFILEWATCHER_H

#include "dfmplugin_recent_global.h"
#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_recent {

class RecentFileWatcherPrivate;
class RecentFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit RecentFileWatcher() = delete;
    explicit RecentFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~RecentFileWatcher() override;
    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true) override;

private:
    void addWatcher(const QUrl &url);
    void removeWatcher(const QUrl &url);
    QUrl getRealUrl(const QUrl &url);

    void onFileDeleted(const QUrl &url);
    void onFileAttributeChanged(const QUrl &url);
    void onFileRename(const QUrl &oldUrl, const QUrl &newUrl);

    RecentFileWatcherPrivate *dptr;
};
}

#endif   // RECENTFILEWATCHER_H
