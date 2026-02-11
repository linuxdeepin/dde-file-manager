// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALAPPENDCOMPRESSPLUGIN_H
#define VIRTUALAPPENDCOMPRESSPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "appendcompress/appendcompresseventreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualAppendCompressPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    DPF_EVENT_REG_HOOK(hook_AppendCompress_Prohibit)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<AppendCompressEventReceiver> eventReceiver { new AppendCompressEventReceiver };
};

}   // namespace dfmplugin_utils

#endif   // VIRTUALAPPENDCOMPRESSPLUGIN_H
