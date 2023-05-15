// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizerplugin.h"
#include "framemanager.h"

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

DFMBASE_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE

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

TEST(OrganizerPlugin, start)
{
    OrganizerPlugin plugin;
    stub_ext::StubExt stub;
    bool init = false;
    stub.set_lamda(&FrameManager::initialize, [&init](){
        init = true;
        return true;
    });
    EXPECT_EQ(plugin.instance, nullptr);

    EXPECT_TRUE(plugin.start());
    ASSERT_NE(plugin.instance, nullptr);
    EXPECT_TRUE(init);

    plugin.stop();
    EXPECT_EQ(plugin.instance, nullptr);
}
