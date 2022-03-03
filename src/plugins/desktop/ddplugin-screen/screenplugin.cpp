/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "screenplugin.h"
#include "screenproxyqt.h"
#include "screenproxydbus.h"

#include <dfm-base/utils/windowutils.h>

#include <services/desktop/screen/screenservice.h>

DFMBASE_USE_NAMESPACE
DDP_SCREEN_USE_NAMESPACE
DSB_D_USE_NAMESPACE

void ScreenPlugin::initialize()
{

}

bool ScreenPlugin::start()
{
    if (WindowUtils::isWayLand())
        proxy = new ScreenProxyDBus();
    else
        proxy = new ScreenProxyQt();

    auto &ctx = dpfInstance.serviceContext();

    // start screen service.
    {
        QString error;
        bool ret = ctx.load(ScreenService::name(), &error);
        Q_ASSERT_X(ret, "ScreenPlugin", error.toStdString().c_str());
    }

    auto service = ctx.service<ScreenService>(ScreenService::name());
    Q_ASSERT_X(service, "ScreenPlugin", "ScreenService not found");

    // find serviceprivate
    auto ptr = service->findChild<QObject *>("dfm_service_desktop::ScreenServicePrivate");
    if (!ptr) {
        qCritical() << "can not find dfm_service_desktop::ScreenServicePrivate.";
        abort();
        return false;
    }

    QMetaObject::invokeMethod(ptr, "setProxy", Qt::DirectConnection, Q_ARG(QObject *, proxy));

    proxy->reset();
    return true;
}

dpf::Plugin::ShutdownFlag ScreenPlugin::stop()
{
    delete proxy;
    proxy = nullptr;
    return kSync;
}
