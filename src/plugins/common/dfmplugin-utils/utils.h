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

DPUTILS_BEGIN_NAMESPACE

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

    // TODO(xust) some part should be refactor.
    // *******************begin UserShare*******************
    // slots
    DPF_EVENT_REG_SLOT(slot_UserShare_StartSmbd)
    DPF_EVENT_REG_SLOT(slot_UserShare_IsSmbdRunning)
    DPF_EVENT_REG_SLOT(slot_UserShare_SetSmbPasswd)
    DPF_EVENT_REG_SLOT(slot_UserShare_AddShare)
    DPF_EVENT_REG_SLOT(slot_UserShare_RemoveShare)
    DPF_EVENT_REG_SLOT(slot_UserShare_IsPathShared)
    DPF_EVENT_REG_SLOT(slot_UserShare_GetAllShareInfos)
    DPF_EVENT_REG_SLOT(slot_UserShare_GetShareInfoOfFilePath)
    DPF_EVENT_REG_SLOT(slot_UserShare_GetShareInfoOfShareName)
    DPF_EVENT_REG_SLOT(slot_UserShare_GetShareNameOfFilePath)
    DPF_EVENT_REG_SLOT(slot_UserShare_GetCurrentUserName)
    DPF_EVENT_REG_SLOT(slot_UserShare_GetWhoSharedByShareName)

    // signals
    DPF_EVENT_REG_SIGNAL(signal_UserShare_ShareCountChanged)
    DPF_EVENT_REG_SIGNAL(signal_UserShare_ShareAdded)
    DPF_EVENT_REG_SIGNAL(signal_UserShare_ShareRemoved)
    DPF_EVENT_REG_SIGNAL(signal_UserShare_RemoveShareFailed)
    // *******************end UserShare*******************

    // *******************begin Bluetooth*******************
    // slots
    DPF_EVENT_REG_SLOT(slot_Bluetooth_IsAvailable)
    DPF_EVENT_REG_SLOT(slot_Bluetooth_SendFiles)
    // *******************end Bluetooth*******************
public:
    virtual void initialize() override;
    virtual bool start() override;
};

DPUTILS_END_NAMESPACE

#endif   // UTILS_H
