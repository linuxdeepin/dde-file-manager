// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEWATCHER_H
#define AVFSFILEWATCHER_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_avfsbrowser {

class AvfsFileWatcherPrivate;
class AvfsFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT

public:
    explicit AvfsFileWatcher(const QUrl &url, QObject *parent = nullptr);
    virtual ~AvfsFileWatcher();
};

}

#endif   // AVFSFILEWATCHER_H
