// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIAFILEWATCHER_P_H
#define MASTEREDMEDIAFILEWATCHER_P_H

#include "dfmplugin_optical_global.h"
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

#include <QPointer>

namespace dfmplugin_optical {

class MasteredMediaFileWatcher;
class MasteredMediaFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend MasteredMediaFileWatcher;

public:
    explicit MasteredMediaFileWatcherPrivate(const QUrl &fileUrl, MasteredMediaFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;

    QString curMnt;
    AbstractFileWatcherPointer proxyStaging;
    AbstractFileWatcherPointer proxyOnDisk;
};

}

#endif   // MASTEREDMEDIAFILEWATCHER_P_H
