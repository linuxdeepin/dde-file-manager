#include "vaultfilewatcherprivate.h"
#include "utils/vaultfilewatcher.h"

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

VaultFileWatcherPrivate::VaultFileWatcherPrivate(const QUrl &fileUrl, VaultFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool VaultFileWatcherPrivate::start()
{
    return proxyStaging && proxyStaging->startWatcher();
}

bool VaultFileWatcherPrivate::stop()
{
    return proxyStaging && proxyStaging->stopWatcher();
}
