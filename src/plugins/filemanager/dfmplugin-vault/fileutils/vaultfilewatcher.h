// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEWATCHER_H
#define VAULTFILEWATCHER_H

#include "dfmplugin_vault_global.h"
#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_vault {
class VaultFileWatcherPrivate;
class VaultFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit VaultFileWatcher() = delete;
    explicit VaultFileWatcher(const QUrl &url, QObject *parent = nullptr);
    virtual ~VaultFileWatcher() override;

private slots:
    void onFileDeleted(const QUrl &url);
    void onFileAttributeChanged(const QUrl &url);
    void onFileRename(const QUrl &fromUrl, const QUrl &toUrl);
    void onSubfileCreated(const QUrl &url);

private:
    VaultFileWatcherPrivate *dptr { nullptr };
};
}
#endif   // VAULTFILEWATCHER_H
