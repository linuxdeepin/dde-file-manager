// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREWATCHER_P_H
#define SHAREWATCHER_P_H

#include "dfmplugin_myshares_global.h"

#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_myshares {

class ShareWatcher;
class ShareWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend class ShareWatcher;

public:
    ShareWatcherPrivate(const QUrl &fileUrl, ShareWatcher *qq);

    virtual bool start() override;
    virtual bool stop() override;
};

}

#endif   // SHAREWATCHER_P_H
