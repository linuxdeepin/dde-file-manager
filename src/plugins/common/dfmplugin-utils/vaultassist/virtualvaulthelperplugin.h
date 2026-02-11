// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALVAULTHELPERPLUGIN_H
#define VIRTUALVAULTHELPERPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "vaulthelperreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {
class VirtualVaultHelperPlugin : public dpf::Plugin
{
    Q_OBJECT

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<VaultHelperReceiver> eventReceiver { new VaultHelperReceiver};
};

}

#endif   // VIRTUALVAULTHELPERPLUGIN_H
