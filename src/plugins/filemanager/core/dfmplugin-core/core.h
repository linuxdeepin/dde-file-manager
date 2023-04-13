// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include "dfmplugin_core_global.h"

#include <dfm-framework/dpf.h>

DPCORE_BEGIN_NAMESPACE
class Core : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "core.json")

    DPF_EVENT_NAMESPACE(DPCORE_NAMESPACE)
    DPF_EVENT_REG_SIGNAL(signal_StartApp)

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;
    void connectToServer();

private slots:
    void onAllPluginsInitialized();
    void onAllPluginsStarted();
    void onWindowOpened(quint64 windd);
};
DPCORE_END_NAMESPACE

#endif   // COREPLUGIN_H
