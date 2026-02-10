// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHFILEWATCHER_P_H
#define SEARCHFILEWATCHER_P_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

class SearchFileWatcher;
class SearchFileWatcherPrivate : public AbstractFileWatcherPrivate
{
    friend class SearchFileWatcher;

public:
    explicit SearchFileWatcherPrivate(const QUrl &fileUrl, SearchFileWatcher *qq);

private:
    bool start() override;
    bool stop() override;

    QHash<QUrl, AbstractFileWatcherPointer> urlToWatcherHash;
};

}

#endif   // SEARCHFILEWATCHER_P_H
