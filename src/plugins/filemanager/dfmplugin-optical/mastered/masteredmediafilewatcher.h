// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIAFILEWATCHER_H
#define MASTEREDMEDIAFILEWATCHER_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_optical {

class MasteredMediaFileWatcherPrivate;
class MasteredMediaFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    friend MasteredMediaFileWatcherPrivate;
    Q_OBJECT

public:
    explicit MasteredMediaFileWatcher(const QUrl &url, QObject *parent = nullptr);

private slots:
    void onFileDeleted(const QUrl &url);
    void onMountPointDeleted(const QString &id);
    void onFileAttributeChanged(const QUrl &url);
    void onFileRename(const QUrl &fromUrl, const QUrl &toUrl);
    void onSubfileCreated(const QUrl &url);

private:
    MasteredMediaFileWatcherPrivate *dptr;
};

}

#endif   // MASTEREDMEDIAFILEWATCHER_H
