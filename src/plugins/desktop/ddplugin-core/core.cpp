/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "core.h"
#include "screenproxydbus.h"
#include "screenproxyqt.h"

#include "services/common/menu/menuservice.h"

#include "wallpaperservice.h"
#include "screenservice.h"
#include "backgroundservice.h"
#include "canvasservice.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/file/local/localfilemenu.h"
#include "dfm-base/utils/clipboard.h"

#include <dfm-framework/framework.h>

DSC_USE_NAMESPACE
DSB_D_USE_NAMESPACE

void registerAllService()
{
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(ScreenService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(BackgroundService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    if (!ctx.load(CanvasService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

void registerFileSystem()
{
    UrlRoute::regScheme(SchemeTypes::kFile, "/");

    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kFile);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kFile);
    MenuService::regClass<LocalFileMenu>(SchemeTypes::kFile);
    // 初始化剪切板
    ClipBoard::instance();
}

void Core::initialize()
{
    registerFileSystem();
    registerAllService();

    auto &ctx = dpfInstance.serviceContext();
    ScreenService *screenService = ctx.service<ScreenService>(ScreenService::name());
    screenService->screens();

    //获取所有的背景
}

bool Core::start()
{
    // qInfo() << PlatformTypes::XCB << ScreenService::instance()->allScreen(PlatformTypes::XCB);
    // qInfo() << PlatformTypes::WAYLAND << ScreenService::instance()->allScreen(PlatformTypes::WAYLAND);

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return kSync;
}
