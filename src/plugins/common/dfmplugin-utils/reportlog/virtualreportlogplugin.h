/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
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
