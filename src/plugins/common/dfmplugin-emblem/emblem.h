/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#ifndef EMBLEM_H
#define EMBLEM_H

#include "dfmplugin_emblem_global.h"

#include <dfm-framework/dpf.h>

DPEMBLEM_BEGIN_NAMESPACE

class Emblem : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "emblem.json")

    DPF_EVENT_NAMESPACE(DPEMBLEM_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_FileEmblems_Paint)

    DPF_EVENT_REG_HOOK(hook_CustomEmblems_Fetch)
    DPF_EVENT_REG_HOOK(hook_ExtendEmblems_Fetch)

public:
    virtual void initialize() override;
    virtual bool start() override;
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEM_H
