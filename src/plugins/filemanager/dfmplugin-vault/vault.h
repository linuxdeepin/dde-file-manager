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
#ifndef VAULT_H
#define VAULT_H

#include "dfmplugin_vault_global.h"

#include <dfm-framework/framework.h>

DPVAULT_BEGIN_NAMESPACE
class Vault : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "vault.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

public slots:
    void onWindowOpened(quint64 winID);
    void onAllPluginsInitialized();

private:
    void addSideBarVaultItem();
    void addCustomCrumbar();
    void addComputer();
    void addFileOperations();
};
DPVAULT_END_NAMESPACE

#endif   // VAULT_H
