// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGFILEWATCHER_P_H
#define TAGFILEWATCHER_P_H

#include "dfmplugin_tag_global.h"
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_tag {

class TagFileWatcher;
class TagFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend class TagFileWatcher;

public:
    TagFileWatcherPrivate(const QUrl &fileUrl, TagFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;

    void initFileWatcher();
    void initConnect();

    AbstractFileWatcherPointer proxy;
    QMap<QUrl, AbstractFileWatcherPointer> urlToWatcherMap;
};

}

#endif   // TAGFILEWATCHER_P_H
