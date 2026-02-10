// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEWATCHER_H
#define TRASHFILEWATCHER_H

#include "dfmplugin_trash_global.h"
#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_trash {

class TrashFileWatcherPrivate;
class TrashFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    friend TrashFileWatcherPrivate;
    Q_OBJECT

public:
    explicit TrashFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~TrashFileWatcher() override;

private:
    TrashFileWatcher() = delete;
    TrashFileWatcherPrivate *dptr;
};

}

#endif   // TRASHFILEWATCHER_H
