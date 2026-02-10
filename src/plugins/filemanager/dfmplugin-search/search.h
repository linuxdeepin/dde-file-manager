// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCH_H
#define SEARCH_H

#include "dfmplugin_search_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_search {

class Search : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "search.json")

    DPF_EVENT_NAMESPACE(DPSEARCH_NAMESPACE)
    // slot events
    DPF_EVENT_REG_SLOT(slot_Custom_Register)
    DPF_EVENT_REG_SLOT(slot_Custom_IsDisableSearch)
    DPF_EVENT_REG_SLOT(slot_Custom_RedirectedPath)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_Commit)

    // hook events
    DPF_EVENT_REG_HOOK(hook_Url_IsSubFile)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 windId);
    void regSearchCrumbToTitleBar();
    void regSearchToWorkspace();
    void regSearchSettingConfig();
    void bindEvents();
    void bindWindows();
};

}

#endif   // SEARCH_H
