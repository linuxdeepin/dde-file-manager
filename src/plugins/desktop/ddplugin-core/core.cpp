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
#include "frame/windowframe.h"

#include "services/desktop/frame/frameservice.h"
#include "services/desktop/screen/screenservice.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/file/local/localfilewatcher.h"

DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

static void registerFileSystem()
{
    UrlRoute::regScheme(Global::kFile, "/");
    InfoFactory::regClass<LocalFileInfo>(Global::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::kFile);
}

void ddplugin_core::Core::initialize()
{
    registerFileSystem();
    connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted, this, &Core::onStart);
}

bool ddplugin_core::Core::start()
{
    // 手动初始化application
    new DFMBASE_NAMESPACE::Application();

    auto &ctx = dpfInstance.serviceContext();

    {
        QString error;
        bool ret = ctx.load(FrameService::name(), &error);
        Q_ASSERT_X(ret, "Core", error.toStdString().c_str());
    }

    auto service = ctx.service<FrameService>(FrameService::name());
    Q_ASSERT_X(service, "Core", "FrameService not found");

    // find serviceprivate
    auto ptr = service->findChild<QObject *>("dfm_service_desktop::FrameServicePrivate");
    if (!ptr) {
        qCritical() << "can not find dfm_service_desktop::FrameServicePrivate.";
        abort();
    }

    frame = new WindowFrame();
    QMetaObject::invokeMethod(ptr, "setProxy", Qt::DirectConnection, Q_ARG(QObject *, frame));

    return true;
}

dpf::Plugin::ShutdownFlag ddplugin_core::Core::stop()
{
    delete frame;
    frame = nullptr;
    return kSync;
}

void ddplugin_core::Core::onStart()
{
    bool ret = frame->init();
    Q_ASSERT_X(ret, "Core", "Fail to init WindowsFrame");

    // create desktop frame windows.
    frame->buildBaseWindow();
}
