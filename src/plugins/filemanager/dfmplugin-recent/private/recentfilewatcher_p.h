// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDIRITERATORPRIVATE_H
#define RECENTDIRITERATORPRIVATE_H

#include "dfmplugin_recent_global.h"
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_recent {

class RecentFileWatcher;
class RecentFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend RecentFileWatcher;

public:
    RecentFileWatcherPrivate(const QUrl &fileUrl, RecentFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;

    void initFileWatcher();
    void initConnect();

    AbstractFileWatcherPointer proxy;
    QMap<QUrl, AbstractFileWatcherPointer> urlToWatcherMap;
};

}
#endif   // RECENTDIRITERATORPRIVATE_H
