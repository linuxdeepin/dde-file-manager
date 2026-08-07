// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_d0_batch.cpp
 * @brief Batch coverage for D0 destructors across many dfm-base classes.
 *        Each test heap-allocates an instance and deletes it, exercising the
 *        D0 (deleting) virtual destructor path.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QUrl>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QFile>

#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localfileiconprovider.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractscenecreator.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/proxyfileinfo.h>
#include <dfm-base/utils/elidetextlayout.h>
#include <dfm-base/utils/highlightprovider.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/systemservicemanager.h>
#include <dfm-base/utils/loggerrules.h>
#include <dfm-base/utils/signalhandler.h>
#include <dfm-base/utils/watchercache.h>
#include <dfm-base/utils/fileinfoasycworker.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>

using namespace dfmbase;

// --- D0 destructor batch tests ---

TEST(D0BatchTest, LoggerRulesD0) {
    auto *p = new LoggerRules(); delete p;
}
TEST(D0BatchTest, SignalHandlerD0) {
    auto *p = new SignalHandler(); delete p;
}
TEST(D0BatchTest, FileInfoHelperD0) {
    EXPECT_NO_FATAL_FAILURE({ (void)FileInfoHelper::instance(); });
}

TEST(D0BatchTest, AsyncFileInfoD0)    { auto *p = new AsyncFileInfo(QUrl::fromLocalFile("/tmp/t")); delete p; }
TEST(D0BatchTest, LocalFileIconProvider) { auto *p = new LocalFileIconProvider(); delete p; }
TEST(D0BatchTest, FileInfoD0)         { auto *p = new FileInfo(QUrl::fromLocalFile("/tmp/t")); delete p; }
TEST(D0BatchTest, AbstractSceneCreatorD0) {
    class F : public AbstractSceneCreator { public: AbstractMenuScene* create() override { return nullptr; } };
    auto *p = new F(); delete p;
}
TEST(D0BatchTest, ElideTextLayoutD0)  { auto *p = new ElideTextLayout("test"); delete p; }
TEST(D0BatchTest, HighlightProviderD0) { auto *p = new HighlightProvider(); delete p; }
TEST(D0BatchTest, SystemPathUtilD0)    { auto *p = new SystemPathUtil(); delete p; }
TEST(D0BatchTest, SystemServiceManagerD0) { 
    auto *p = new SystemServiceManager(); delete p;
}
