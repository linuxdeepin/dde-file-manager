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
#include "delegateservice.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

#include <QUrl>

DSC_USE_NAMESPACE

void DelegateService::registerTransparentHandle(const DelegateService::KeyType &scheme, const DelegateService::TransparentHandle &handle)
{
    if (isRegistedTransparentHandle(scheme))
        return;

    transparentHandles.insert(scheme, handle);
}

bool DelegateService::isRegistedTransparentHandle(const DelegateService::KeyType &scheme) const
{
    return transparentHandles.contains(scheme);
}

DelegateService::TransparentHandleMap DelegateService::getTransparentHandles()
{
    return transparentHandles;
}

bool DelegateService::isTransparent(const QUrl &url)
{
    const QString &scheme { url.scheme() };

    if (!transparentHandles.contains(scheme))
        return false;

    return transparentHandles.value(scheme)(url);
}

void DelegateService::registerUrlTransform(const DelegateService::KeyType &scheme, const UrlTransformHandle &handle)
{
    if (isRegisterUrlTransform(scheme))
        return;

    urlTransformHandles.insert(scheme, handle);
}

bool DelegateService::isRegisterUrlTransform(const DelegateService::KeyType &scheme)
{
    return urlTransformHandles.contains(scheme);
}

QUrl DelegateService::urlTransform(const QUrl &url)
{
    const QString &scheme { url.scheme() };
    if (!isRegisterUrlTransform(scheme))
        return url;

    return urlTransformHandles.value(scheme)(url);
}

DelegateService *DelegateService::instance()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::DelegateService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::DelegateService>(DSC_NAMESPACE::DelegateService::name());
}

DelegateService::DelegateService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DelegateService>()
{
}
