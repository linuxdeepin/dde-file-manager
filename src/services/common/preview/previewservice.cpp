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
#include "previewservice.h"
#include "preview_defines.h"
#include "dfm-base/utils/universalutils.h"

#include <mutex>

DSC_BEGIN_NAMESPACE
namespace Preview {
namespace EventType {
const int kShowPreviewEvent = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}
}
DSC_END_NAMESPACE

DSC_USE_NAMESPACE
PreviewService::PreviewService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<PreviewService>()
{
}

PreviewService *PreviewService::instance()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::PreviewService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::PreviewService>(DSC_NAMESPACE::PreviewService::name());
}
