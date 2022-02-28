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

#include <dfm-framework/framework.h>

DPSEARCH_BEGIN_NAMESPACE

class Search : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "search.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;
    void subscribeEvent();

private slots:
    void onWindowOpened(quint64 windId);
    void regSearchCrumbToTitleBar();
    void regSearchToWorkspaceService();
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCH_H
