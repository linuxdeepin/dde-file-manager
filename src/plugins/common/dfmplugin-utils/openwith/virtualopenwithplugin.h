// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALOPENWITHPLUGIN_H
#define VIRTUALOPENWITHPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "openwith/openwitheventreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualOpenWithPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    DPF_EVENT_REG_SLOT(slot_OpenWith_ShowDialog)

    DPF_EVENT_REG_HOOK(hook_OpenWith_DisabledOpenWithWidget)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<OpenWithEventReceiver> eventReceiver { new OpenWithEventReceiver };
    void regViewToPropertyDialog();
};

}   // namespace dfmplugin_utils

#endif   // VIRTUALOPENWITHPLUGIN_H
