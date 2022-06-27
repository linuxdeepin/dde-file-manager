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
#include "opticaleventcaller.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_optical;
DFMBASE_USE_NAMESPACE

void OpticalEventCaller::sendOpenBurnDlg(const QString &dev, bool isSupportedUDF, QWidget *parent)
{
    dpfSlotChannel->push("dfmplugin_burn", "slot_ShowBurnDialog", dev, isSupportedUDF, parent);
}
