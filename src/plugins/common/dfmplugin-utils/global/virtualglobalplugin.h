// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALGLOBALPLUGIN_H
#define VIRTUALGLOBALPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "global/globaleventreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualGlobalPlugin : public dpf::Plugin
{
    Q_OBJECT

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<GlobalEventReceiver> eventReceiver { new GlobalEventReceiver };
};

}   // dfmplugin_utils

#endif   // VIRTUALGLOBALPLUGIN_H
