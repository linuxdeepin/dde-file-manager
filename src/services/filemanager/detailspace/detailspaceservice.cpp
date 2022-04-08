/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "detailspaceservice.h"
#include "private/detailspaceservice_p.h"
#include "dfm-base/utils/universalutils.h"

DSB_FM_BEGIN_NAMESPACE
namespace DetailEventType {
const int kShowDetailView = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
const int kSetDetailViewSelectFileUrl = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}
DSB_FM_END_NAMESPACE

DSB_FM_USE_NAMESPACE

/*!
 * \class DetailSpaceServicePrivate
 * \brief
 */

DetailSpaceServicePrivate::DetailSpaceServicePrivate(DetailSpaceService *serv)
    : QObject(nullptr), service(serv)
{
}

/*!
 * \class DetailSpaceService
 * \brief
 */

DetailSpaceService *DetailSpaceService::serviceInstance()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::DetailSpaceService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::DetailSpaceService>(DSB_FM_NAMESPACE::DetailSpaceService::name());
}

DetailSpaceService::DetailSpaceService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DetailSpaceService>(),
      d(new DetailSpaceServicePrivate(this))
{
}

DetailSpaceService::~DetailSpaceService()
{
}

bool DetailSpaceService::registerMethod(DTSP_NAMESPACE::RegisterExpandProcess::createControlViewFunc view, int index, QString *error)
{
    return DTSP_NAMESPACE::RegisterExpandProcess::instance()->registerFunction(view, index, error);
}

QMap<int, QWidget *> DetailSpaceService::createControlView(const QUrl &url)
{
    return DTSP_NAMESPACE::RegisterExpandProcess::instance()->createControlView(url);
}
