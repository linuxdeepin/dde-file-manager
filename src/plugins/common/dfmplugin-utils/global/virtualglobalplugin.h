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
#ifndef VIRTUALGLOBALPLUGIN_H
#define VIRTUALGLOBALPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "global/globaleventreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualGlobalPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    DPF_EVENT_REG_HOOK(hook_UrlsTransform)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<GlobalEventReceiver> eventReceiver { new GlobalEventReceiver };
};

}   // dfmplugin_utils

#endif   // VIRTUALGLOBALPLUGIN_H
