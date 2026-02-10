// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREWATCHER_H
#define SHAREWATCHER_H

#include "dfmplugin_myshares_global.h"

#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_myshares {

class ShareWatcherPrivate;
class ShareWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT
    friend class ShareWatcherPrivate;

public:
    explicit ShareWatcher(const QUrl &url, QObject *parent = nullptr);
    virtual ~ShareWatcher() override;

    void shareAdded(const QString &path);
    void shareRemoved(const QString &path);
};

}

#endif   // SHAREWATCHER_H
