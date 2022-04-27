#ifndef VAULTFILEWATCHERPRIVATE_H
#define VAULTFILEWATCHERPRIVATE_H

#include "dfmplugin_vault_global.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/interfaces/private/abstractfilewatcher_p.h"

DPVAULT_BEGIN_NAMESPACE
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
DPVAULT_END_NAMESPACE
#endif   // VAULTFILEWATCHERPRIVATE_H
