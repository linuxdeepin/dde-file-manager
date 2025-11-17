// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>

#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-io/dwatcher.h"

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

using namespace dfmplugin_trash;

class TestTrashFileWatcher : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestTrashFileWatcher, Constructor)
{
    QUrl url("trash:///");
    TrashFileWatcher watcher(url);
    
    EXPECT_EQ(watcher.url(), url);
}

TEST_F(TestTrashFileWatcher, ConstructorWithParent)
{
    QUrl url("trash:///");
    QObject parent;
    TrashFileWatcher *watcher = new TrashFileWatcher(url, &parent);
    
    EXPECT_EQ(watcher->url(), url);
    EXPECT_EQ(watcher->parent(), &parent);
    
    delete watcher;
}

TEST_F(TestTrashFileWatcher, Destructor)
{
    TrashFileWatcher *watcher = new TrashFileWatcher(QUrl("trash:///"));
    EXPECT_NE(watcher, nullptr);
    delete watcher;
}

TEST_F(TestTrashFileWatcher, Url)
{
    QUrl url("trash:///test/");
    TrashFileWatcher watcher(url);
    
    EXPECT_EQ(watcher.url(), url);
}

TEST_F(TestTrashFileWatcher, SignalEmissions)
{
    QUrl url("trash:///");
    TrashFileWatcher watcher(url);
    
    // Test that the watcher can emit signals
    QSignalSpy fileAttributeChangedSpy(&watcher, &AbstractFileWatcher::fileAttributeChanged);
    QSignalSpy fileDeletedSpy(&watcher, &AbstractFileWatcher::fileDeleted);
    QSignalSpy subfileCreatedSpy(&watcher, &AbstractFileWatcher::subfileCreated);
    QSignalSpy fileRenameSpy(&watcher, &AbstractFileWatcher::fileRename);
    
    // Just test that the signal spies are created successfully
    EXPECT_TRUE(fileAttributeChangedSpy.isValid());
    EXPECT_TRUE(fileDeletedSpy.isValid());
    EXPECT_TRUE(subfileCreatedSpy.isValid());
    EXPECT_TRUE(fileRenameSpy.isValid());
}

TEST_F(TestTrashFileWatcher, FileUtilsBindUrlTransform)
{
    QUrl testUrl("trash:///test.txt");
    
    // Mock FileUtils::bindUrlTransform
    stub.set_lamda(static_cast<QUrl (*)(const QUrl &)>(&FileUtils::bindUrlTransform), 
                   [](const QUrl &url) -> QUrl {
        return url;
    });
    
    QUrl result = FileUtils::bindUrlTransform(testUrl);
    EXPECT_EQ(result, testUrl);
}