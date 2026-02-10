// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHFILEWATCHER_H
#define SEARCHFILEWATCHER_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_search {

class SearchFileWatcherPrivate;
class SearchFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit SearchFileWatcher() = delete;
    explicit SearchFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~SearchFileWatcher() override;
    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true) override;

private:
    void addWatcher(const QUrl &url);
    void removeWatcher(const QUrl &url);

    void onFileDeleted(const QUrl &url);
    void onFileAttributeChanged(const QUrl &url);
    void onFileRenamed(const QUrl &fromUrl, const QUrl &toUrl);
    void onFileAdd(const QUrl &url);

private Q_SLOTS:
    void handleFileAdd(const QUrl &url);
    void handleFileDelete(const QUrl &url);
    void handleFileRename(const QUrl &oldUrl, const QUrl &newUrl);

private:
    SearchFileWatcherPrivate *dptr;
};

}

#endif   // SEARCHFILEWATCHER_H
