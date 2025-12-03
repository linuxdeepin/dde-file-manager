// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QUrl>
#include <QTimer>

#include "stubext.h"
#include "events/trashcoreeventsender.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-framework/dpf.h>

using namespace dfmplugin_trashcore;

class TrashCoreEventSenderTest : public testing::Test
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

TEST_F(TrashCoreEventSenderTest, Instance_Basic)
{
    auto *instance1 = TrashCoreEventSender::instance();
    auto *instance2 = TrashCoreEventSender::instance();
    
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TrashCoreEventSenderTest, Constructor_Basic)
{
    EXPECT_NO_THROW(TrashCoreEventSender::instance());
}

TEST_F(TrashCoreEventSenderTest, CheckAndStartWatcher_Basic)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Mock NetworkUtils::checkAllCIFSBusy to return false
    bool cifsBusy = false;
    stub.set_lamda(&DFMBASE_NAMESPACE::NetworkUtils::checkAllCIFSBusy, [&cifsBusy]() {
        return cifsBusy;
    });
    
    // Simple test that doesn't cause crashes
    EXPECT_NO_THROW(sender->checkAndStartWatcher());
}

TEST_F(TrashCoreEventSenderTest, CheckAndStartWatcher_CIFSBusy)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Mock NetworkUtils::checkAllCIFSBusy to return true
    bool cifsBusy = true;
    stub.set_lamda(&DFMBASE_NAMESPACE::NetworkUtils::checkAllCIFSBusy, [&cifsBusy]() {
        return cifsBusy;
    });
    
    // Simple test that doesn't cause crashes
    EXPECT_NO_THROW(sender->checkAndStartWatcher());
}

TEST_F(TrashCoreEventSenderTest, SendTrashStateChangedDel_EmptyTrash)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Mock FileUtils::trashIsEmpty to return true
    bool trashEmpty = true;
    stub.set_lamda(&DFMBASE_NAMESPACE::FileUtils::trashIsEmpty, [&trashEmpty]() {
        return trashEmpty;
    });
    
    // Since we're not stubbing the signal dispatcher, just ensure it doesn't crash
    EXPECT_NO_THROW(sender->sendTrashStateChangedDel());
}

TEST_F(TrashCoreEventSenderTest, SendTrashStateChangedDel_NonEmptyTrash)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Mock FileUtils::trashIsEmpty to return false
    bool trashEmpty = false;
    stub.set_lamda(&DFMBASE_NAMESPACE::FileUtils::trashIsEmpty, [&trashEmpty]() {
        return trashEmpty;
    });
    
    // Since we're not stubbing the signal dispatcher, just ensure it doesn't crash
    EXPECT_NO_THROW(sender->sendTrashStateChangedDel());
}

TEST_F(TrashCoreEventSenderTest, SendTrashStateChangedAdd_FromEmptyToNonEmpty)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Since we're not stubbing the signal dispatcher, just ensure it doesn't crash
    EXPECT_NO_THROW(sender->sendTrashStateChangedAdd());
}

TEST_F(TrashCoreEventSenderTest, SendTrashStateChangedAdd_FromNotEmpty)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Since we're not stubbing the signal dispatcher, just ensure it doesn't crash
    EXPECT_NO_THROW(sender->sendTrashStateChangedAdd());
}

TEST_F(TrashCoreEventSenderTest, InitTrashWatcher_Basic)
{
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();
    
    // Since the constructor and connections involve complex objects, just ensure it doesn't crash
    EXPECT_NO_THROW(sender->initTrashWatcher());
}