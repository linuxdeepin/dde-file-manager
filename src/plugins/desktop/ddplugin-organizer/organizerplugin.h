/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ORGANIZERPLUGIN_H
#define ORGANIZERPLUGIN_H

#include "ddplugin_organizer_global.h"

#include <dfm-framework/dpf.h>

namespace ddplugin_organizer {

class FrameManager;
class OrganizerPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "organizerplugin.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    FrameManager *instance = nullptr;

private:
    DPF_EVENT_NAMESPACE(DDP_ORGANIZER_NAMESPACE)

    // CollectionView begin
    DPF_EVENT_REG_SLOT(slot_CollectionView_GridPoint)
    DPF_EVENT_REG_SLOT(slot_CollectionView_VisualRect)
    DPF_EVENT_REG_SLOT(slot_CollectionView_View)

    DPF_EVENT_REG_SIGNAL(signal_CollectionView_ReportMenuData)

    // CollectionItemDelegate begin
    DPF_EVENT_REG_SLOT(slot_CollectionItemDelegate_IconRect)

    // CollectionModel begin
    DPF_EVENT_REG_SLOT(slot_CollectionModel_Refresh)
};

}

#endif   // ORGANIZERPLUGIN_H
