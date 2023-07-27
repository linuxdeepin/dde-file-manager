// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vault.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/vaulthelper.h"
#include "events/vaulteventreceiver.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QUrl>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(Qt::DropAction *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

void Vault::initialize()
{
    VaultVisibleManager::instance()->infoRegister();
    VaultEventReceiver::instance()->connectEvent();
    bindWindows();
}

bool Vault::start()
{
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
