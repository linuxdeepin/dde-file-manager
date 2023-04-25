// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasplugin.h"
#include "canvasmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include "stubext.h"

#include <gtest/gtest.h>

#include <QDBusConnection>


DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
    struct GlobalEnv
    {
        GlobalEnv() {
            UrlRoute::regScheme(Global::Scheme::kFile, "/");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
            DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
            WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        }
        DFMBASE_NAMESPACE::Application appCfg;
    };

}
static GlobalEnv gEnv;

TEST(CanvasPlugin, start)
{
    CanvasPlugin cp;
    stub_ext::StubExt stub;
    bool init = false;
    stub.set_lamda(&CanvasManager::init, [&init](){
        init = true;
    });

    QString cpath;
    QString cifs;
    stub.set_lamda((bool (QDBusConnection::*)(const QString &, const QString &, QObject *,
                          QDBusConnection::RegisterOptions))&QDBusConnection::registerObject, [&cpath, &cifs](QDBusConnection *,
                   const QString &path, const QString &interface, QObject *object,
                   QDBusConnection::RegisterOptions options){
        cpath = path;
        cifs = interface;
        return true;
    });

    cp.start();

    EXPECT_TRUE(init);
    EXPECT_NE(cp.proxy, nullptr);

    cp.stop();
    EXPECT_EQ(cp.proxy, nullptr);
}
