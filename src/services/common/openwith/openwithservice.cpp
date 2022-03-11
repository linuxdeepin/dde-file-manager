/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "openwithservice.h"
#include "openwith_defines.h"
#include "private/openwithdialog.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-framework/framework.h>

DSC_BEGIN_NAMESPACE

namespace OpenWith {
namespace EventType {
const int kOpenWith = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}
}
DSC_END_NAMESPACE

DFMBASE_USE_NAMESPACE
DSC_BEGIN_NAMESPACE

OpenWithService *OpenWithService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(name()))
            abort();
    });

    return ctx.service<OpenWithService>(name());
}

void OpenWithService::showOpenWithDialog(const QList<QUrl> &urls)
{
    OpenWithDialog *d = new OpenWithDialog(urls);
    d->setDisplayPosition(OpenWithDialog::Center);
    d->open();
}

OpenWithService::OpenWithService(QObject *parent)
    : dpf::PluginService(parent), dpf::AutoServiceRegister<OpenWithService>()
{
    dpfInstance.eventDispatcher().subscribe(OpenWith::EventType::kOpenWith, this, static_cast<void (OpenWithService::*)(const QList<QUrl> &)>(&OpenWithService::showOpenWithDialog));
}

OpenWithService::~OpenWithService()
{
}

DSC_END_NAMESPACE
