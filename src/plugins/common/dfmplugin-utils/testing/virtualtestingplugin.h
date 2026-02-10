// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALTESTINGPLUGIN_H
#define VIRTUALTESTINGPLUGIN_H

#include "dfmplugin_utils_global.h"

#include <dfm-framework/dpf.h>

#include <QObject>

DPUTILS_BEGIN_NAMESPACE

class VirtualTestingPlugin : public dpf::Plugin
{
    Q_OBJECT

    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_Accessible_SetAccessibleName)
    DPF_EVENT_REG_SLOT(slot_Accessible_SetObjectName)
public:
    virtual void initialize() override;
    virtual bool start() override;


};

DPUTILS_END_NAMESPACE

#endif // VIRTUALTESTINGPLUGIN_H
