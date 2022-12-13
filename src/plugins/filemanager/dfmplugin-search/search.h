/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 windId);
    void regSearchCrumbToTitleBar();
    void regSearchToWorkspace();
    void regSearchToDetailView();
    void bindEvents();
    void bindWindows();
};

}

#endif   // SEARCH_H
