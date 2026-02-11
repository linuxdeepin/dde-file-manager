// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALREPORTLOGPLUGIN_H
#define VIRTUALREPORTLOGPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "reportlog/reportlogeventreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualReportLogPlugin : public dpf::Plugin
{
    Q_OBJECT
public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<ReportLogEventReceiver> eventReceiver { new ReportLogEventReceiver };
};

}   // namespace dfmplugin_utils

#endif   // VIRTUALREPORTLOGPLUGIN_H
