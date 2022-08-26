/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef DETAILSPACE_H
#define DETAILSPACE_H

#include "dfmplugin_propertydialog_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_propertydialog {

class PropertyDialog : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "propertydialog.json")

    DPF_EVENT_NAMESPACE(DPPROPERTYDIALOG_NAMESPACE)

    // slot events
    DPF_EVENT_REG_SLOT(slot_PropertyDialog_Show)
    DPF_EVENT_REG_SLOT(slot_ViewExtension_Register)
    DPF_EVENT_REG_SLOT(slot_ViewExtension_Unregister)
    DPF_EVENT_REG_SLOT(slot_CustomView_Register)
    DPF_EVENT_REG_SLOT(slot_CustomView_UnRegister)
    DPF_EVENT_REG_SLOT(slot_BasicViewExtension_Register)
    DPF_EVENT_REG_SLOT(slot_BasicViewExtension_Unregister)
    DPF_EVENT_REG_SLOT(slot_BasicFiledFilter_Add)
    DPF_EVENT_REG_SLOT(slot_BasicFiledFilter_Remove)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // DETAILSPACE_H
