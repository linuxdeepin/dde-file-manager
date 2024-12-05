// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPTMANAGER_H
#define ENCRYPTMANAGER_H

#include "encrypt_manager_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_encrypt_manager {

class EncryptManager : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "encryptmanager.json")

    DPF_EVENT_NAMESPACE(DPENCRYPTMANAGER_NAMESPACE)
    // slot events
    DPF_EVENT_REG_SLOT(slot_TPMIsAvailable)
    DPF_EVENT_REG_SLOT(slot_GetRandomByTPM)
    DPF_EVENT_REG_SLOT(slot_IsTPMSupportAlgo)
    DPF_EVENT_REG_SLOT(slot_EncryptByTPM)
    DPF_EVENT_REG_SLOT(slot_DecryptByTPM)
    DPF_EVENT_REG_SLOT(slot_TPMIsAvailablePro)
    DPF_EVENT_REG_SLOT(slot_GetRandomByTPMPro)
    DPF_EVENT_REG_SLOT(slot_IsTPMSupportAlgoPro)
    DPF_EVENT_REG_SLOT(slot_EncryptByTPMPro)
    DPF_EVENT_REG_SLOT(slot_DecryptByTPMPro)
    DPF_EVENT_REG_SLOT(slot_OwnerAuthStatus)

public:
    void initialize() override;
    bool start() override;
};

}

#endif   // ENCRYPTMANAGER_H
