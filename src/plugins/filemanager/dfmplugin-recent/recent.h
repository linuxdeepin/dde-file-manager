// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTPLUGIN_H
#define RECENTPLUGIN_H

#include "dfmplugin_recent_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_recent {
class Recent : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "recent.json")

    DPF_EVENT_NAMESPACE(DPRECENT_NAMESPACE)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 windId);
    void regRecentCrumbToTitleBar();
    void regRecentItemToSideBar();

private:
    void updateRecentItemToSideBar();

    void followEvents();
    void bindEvents();
    void bindWindows();
};

}
#endif   // RECENTPLUGIN_H
