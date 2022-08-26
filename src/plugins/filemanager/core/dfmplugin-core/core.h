/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

private slots:
    void onAllPluginsInitialized();
    void onAllPluginsStarted();
};
DPCORE_END_NAMESPACE

#endif   // COREPLUGIN_H
