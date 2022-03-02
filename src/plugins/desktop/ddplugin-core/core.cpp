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

#include <services/desktop/frame/frameservice.h>
#include <services/desktop/screen/screenservice.h>

#include <base/standardpaths.h>
#include <base/schemefactory.h>
#include <file/local/localfileinfo.h>
#include <file/local/localdiriterator.h>
#include <file/local/localfilewatcher.h>

DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

static void registerFileSystem()
{
    UrlRoute::regScheme(SchemeTypes::kFile, "/");
    InfoFactory::regClass<LocalFileInfo>(SchemeTypes::kFile);
    DirIteratorFactory::regClass<LocalDirIterator>(SchemeTypes::kFile);
    WacherFactory::regClass<LocalFileWatcher>(SchemeTypes::kFile);
}

void ddplugin_core::Core::initialize()
{
    registerFileSystem();
    auto &listen = dpfInstance.listener();
    connect(&listen, &DPF_NAMESPACE::Listener::pluginsStarted, this, &Core::onStart);
}

bool ddplugin_core::Core::start()
{
    auto &ctx = dpfInstance.serviceContext();

    {
        QString error;
        Q_ASSERT_X(ctx.load(FrameService::name(), &error), "Core", error.toStdString().c_str());
    }

    auto service = ctx.service<FrameService>(FrameService::name());
    Q_ASSERT_X(service, "Core", "FrameService not found");

    // find serviceprivate
    auto ptr = service->findChild<QObject *>("dfm_service_desktop::FrameServicePrivate");
    if (!ptr) {
        qCritical() << "can not find dfm_service_desktop::FrameServicePrivate.";
        abort();
        return false;
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

