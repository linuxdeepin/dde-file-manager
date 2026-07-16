// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTFILEWATCHER_P_H
#define RECENTFILEWATCHER_P_H

#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_recent {

class RecentFileWatcher;
class RecentFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend RecentFileWatcher;

public:
    explicit RecentFileWatcherPrivate(const QUrl &fileUrl, RecentFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;
};

}

#endif   // RECENTFILEWATCHER_P_H
