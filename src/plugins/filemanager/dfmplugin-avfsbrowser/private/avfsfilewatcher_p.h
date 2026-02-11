// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEWATCHER_P_H
#define AVFSFILEWATCHER_P_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_avfsbrowser {

class AvfsFileWatcher;
class AvfsFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend class AvfsFileWatcher;

public:
    explicit AvfsFileWatcherPrivate(const QUrl &url, AvfsFileWatcher *qq);
};

}

#endif   // AVFSFILEWATCHER_P_H
