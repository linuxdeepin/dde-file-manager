// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULT_H
#define VAULT_H

#include "dfmplugin_vault_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_vault {
class Vault : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "vault.json")

    DPF_EVENT_NAMESPACE(DPVAULT_NAMESPACE)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_Commit)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void bindWindows();
};
}   // namespace dfmplugin_vault

#endif   // VAULT_H
