/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef BURN_H
#define BURN_H

#include "dfmplugin_burn_global.h"

#include <dfm-framework/dpf.h>

DPBURN_BEGIN_NAMESPACE

class Burn : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "burn.json")

    DPF_EVENT_NAMESPACE(DPBURN_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_ShowBurnDialog)
    DPF_EVENT_REG_SLOT(slot_Erase)
    DPF_EVENT_REG_SLOT(slot_PasteTo)

public:
    virtual bool start() override;

private slots:
    void bindScene(const QString &parentScene);
    void bindEvent();
};

DPBURN_END_NAMESPACE

#endif   // BURN_H
