/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "vault.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/vaulthelper.h"

#include <QUrl>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_vault;

void Vault::initialize()
{
    VaultVisibleManager::instance()->infoRegister();
}

bool Vault::start()
{
    VaultVisibleManager::instance()->pluginServiceRegister();

    // follow event
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", VaultHelper::instance(), &VaultHelper::urlsToLocal);

    return true;
}

dpf::Plugin::ShutdownFlag Vault::stop()
{
    return kSync;
}
