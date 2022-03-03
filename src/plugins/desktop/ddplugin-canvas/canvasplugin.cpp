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
#include "canvasplugin.h"
#include "canvasmanager.h"
#include "menu/canvasmenu.h"
#include "utils/fileutil.h"

#include <services/common/menu/menuservice.h>
#include <services/desktop/canvas/canvasservice.h>

#include <dfm-base/utils/clipboard.h>

DSC_USE_NAMESPACE
DSB_D_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void CanvasPlugin::initialize()
{

}

bool CanvasPlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();

    // start screen service.
    {
        QString error;
        bool ret = ctx.load(CanvasService::name(), &error);
        Q_ASSERT_X(ret, "CanvasPlugin", error.toStdString().c_str());
        ret = ctx.load(MenuService::name(), &error);
        Q_ASSERT_X(ret, "CanvasPlugin", error.toStdString().c_str());
    }

    auto service = ctx.service<CanvasService>(CanvasService::name());
    Q_ASSERT_X(service, "CanvasPlugin", "CanvasService not found");

    // find serviceprivate
    auto ptr = service->findChild<QObject *>("dfm_service_desktop::CanvasServicePrivate");
    if (!ptr) {
        qCritical() << "can not find dfm_service_desktop::ScreenServicePrivate.";
        abort();
        return false;
    }

    // initialize file creator
    DesktopFileCreator::instance();

    // 初始化剪切板
    ClipBoard::instance();

    // register canvas menu
    MenuService::regClass<CanvasMenu>(MenuScene::kDesktopMenu);

    proxy = CanvasIns->instance();
    QMetaObject::invokeMethod(ptr, "setProxy", Qt::DirectConnection, Q_ARG(QObject *, proxy));

    proxy->init();
    return true;
}

dpf::Plugin::ShutdownFlag CanvasPlugin::stop()
{
    delete proxy;
    proxy = nullptr;
    return kSync;
}
