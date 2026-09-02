// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEWATCHERPRIVATE_H
#define TRASHFILEWATCHERPRIVATE_H

#include "dfmplugin_trash_global.h"
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_trash {

class TrashFileWatcher;
class TrashFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend TrashFileWatcher;

public:
    TrashFileWatcherPrivate(const QUrl &fileUrl, TrashFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;

    void initFileWatcher();
    void initConnect();

    QList<QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher>> watchers;
};

}
#endif   // TRASHFILEWATCHERPRIVATE_H
