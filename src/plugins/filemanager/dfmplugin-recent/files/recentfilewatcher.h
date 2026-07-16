// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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

private:
    RecentFileWatcherPrivate *dptr;
};

}

#endif   // RECENTFILEWATCHER_H
