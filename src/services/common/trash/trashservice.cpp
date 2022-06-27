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
#include "trashservice.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/schemefactory.h"

#include "dfm-framework/framework.h"

#include <QUrl>

namespace dfm_service_common {

namespace Trash {
namespace EventType {
const int kEmptyTrash = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}
}

}
using namespace dfm_service_common;

TrashService *TrashService::instance()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::TrashService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::TrashService>(DSC_NAMESPACE::TrashService::name());
}

TrashService::TrashService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<TrashService>()
{
}

bool TrashService::isEmpty()
{
    return dpfInstance.eventUnicast().push(DSC_FUNC_NAME).toBool();
}
