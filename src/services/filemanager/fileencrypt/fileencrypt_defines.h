#ifndef VAULT_DEFINES_H
#define VAULT_DEFINES_H

#include "dfm_filemanager_service_global.h"
DSB_FM_BEGIN_NAMESPACE
enum ServiceVaultState {
    kNotExisted,
    kEncrypted,
    kUnlocked,
    kUnderProcess,
    kBroken,
    kNotAvailable
};
DSB_FM_END_NAMESPACE
#endif   // VAULT_DEFINES_H
