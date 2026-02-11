// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vault.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/vaulthelper.h"
#include "utils/vaultentryfileentity.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/operatorcenter.h"
#include "dbus/vaultdbusutils.h"
#include "events/vaulteventreceiver.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QUrl>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(Qt::DropAction *)

DFMBASE_USE_NAMESPACE
namespace dfmplugin_vault {
DFM_LOG_REGISTER_CATEGORY(DPVAULT_NAMESPACE)

void Vault::initialize()
{
    qRegisterMetaType<dfmplugin_vault::VaultEntryFileEntity *>();

    VaultVisibleManager::instance()->infoRegister();
    VaultEventReceiver::instance()->connectEvent();
    VaultHelper::instance();
    FileEncryptHandle::instance();
    OperatorCenter::getInstance();
    VaultDBusUtils::instance();
    bindWindows();
}

bool Vault::start()
{
    QString err;
    if (!DConfigManager::instance()->addConfig(kVaultDConfigName, &err))
        fmWarning() << "Vault: create dconfig failed: " << err;

    VaultVisibleManager::instance()->pluginServiceRegister();
    return true;
}

void Vault::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [](quint64 id) {
        VaultVisibleManager::instance()->onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened,
            VaultVisibleManager::instance(), &VaultVisibleManager::onWindowOpened, Qt::DirectConnection);
}
}   // namespace dfmplugin_vault
