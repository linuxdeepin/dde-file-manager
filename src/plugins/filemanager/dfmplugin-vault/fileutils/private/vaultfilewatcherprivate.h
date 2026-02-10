// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTFILEWATCHERPRIVATE_H
#define VAULTFILEWATCHERPRIVATE_H

#include "dfmplugin_vault_global.h"
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>

namespace dfmplugin_vault {
class VaultFileWatcher;
class VaultFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend VaultFileWatcher;

public:
    explicit VaultFileWatcherPrivate(const QUrl &fileUrl, VaultFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;

    AbstractFileWatcherPointer proxyStaging;
};
}
#endif   // VAULTFILEWATCHERPRIVATE_H
