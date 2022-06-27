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
#ifndef UTILS_H
#define UTILS_H

#include "dfmplugin_utils_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class Utils : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "utils.json")

    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    // *******************begin open with*******************
    DPF_EVENT_REG_SLOT(slot_OpenWith_ShowDialog)
    // *******************end open with*******************

    // *******************begin AppendCompress*******************
    DPF_EVENT_REG_HOOK(hook_AppendCompress_Prohibit)
    // *******************end AppendCompress*******************

    // *******************begin Bluetooth*******************
    // slots
    DPF_EVENT_REG_SLOT(slot_Bluetooth_IsAvailable)
    DPF_EVENT_REG_SLOT(slot_Bluetooth_SendFiles)
    // *******************end Bluetooth*******************

    // *******************begin utils: url to local*****************
    // hook events
    DPF_EVENT_REG_HOOK(hook_UrlsTransform)
    // *******************end  utils: url to local******************

public:
    virtual void initialize() override;
    virtual bool start() override;
};

}

#endif   // UTILS_H
