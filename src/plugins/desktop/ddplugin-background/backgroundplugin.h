// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDPLUGIN_H
#define BACKGROUNDPLUGIN_H

#include "ddplugin_background_global.h"
#include "backgroundmanager.h"

#include <dfm-framework/dpf.h>

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "background.json")

    DPF_EVENT_NAMESPACE(DDP_BACKGROUND_NAMESPACE)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_BackgroundPaint)
    DPF_EVENT_REG_SIGNAL(signal_Background_BackgroundSetted)

    // slot events
    DPF_EVENT_REG_SLOT(slot_FetchUseColorBackground)

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    void bindEvent();

    BackgroundManager *backgroundManager = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif   // BACKGROUNDPLUGIN_H
