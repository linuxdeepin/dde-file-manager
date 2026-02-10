// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticaleventcaller.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_optical;
DFMBASE_USE_NAMESPACE

void OpticalEventCaller::sendOpenBurnDlg(const QString &dev, bool isSupportedUDF, QWidget *parent)
{
    dpfSlotChannel->push("dfmplugin_burn", "slot_BurnDialog_Show", dev, isSupportedUDF, parent);
}

void OpticalEventCaller::sendOpenDumpISODlg(const QString &devId)
{
    dpfSlotChannel->push("dfmplugin_burn", "slot_DumpISODialog_Show", devId);
}
