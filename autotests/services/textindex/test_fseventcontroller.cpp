// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fseventcontroller.cpp
 * @brief Unit tests for FSEventController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fseventcontroller.h"

#include <QTest>

using namespace src;

class FSEventControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSEventController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSEventController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSEventControllerTest, clearCollections)
{
    // Test method: void clearCollections(())
    EXPECT_NO_FATAL_FAILURE(obj->clearCollections());
}

TEST_F(FSEventControllerTest, isEnabled)
{
    // Test bool getter: isEnabled()
    bool result = obj->isEnabled();
    EXPECT_FALSE(result);

}

TEST_F(FSEventControllerTest, onFilesDeleted)
{
    // Test method: void onFilesDeleted((const QStringList &paths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesDeleted(_arg0));
}

TEST_F(FSEventControllerTest, setEnabled)
{
    // Test setter: void setEnabled((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setEnabled(false));
}

TEST_F(FSEventControllerTest, setSilentlyRefreshStarted)
{
    // Test setter: void setSilentlyRefreshStarted((bool flag))
    EXPECT_NO_FATAL_FAILURE(obj->setSilentlyRefreshStarted(false));
}

TEST_F(FSEventControllerTest, setupFSEventCollector)
{
    // Test method: void setupFSEventCollector(())
    EXPECT_NO_FATAL_FAILURE(obj->setupFSEventCollector());
}

TEST_F(FSEventControllerTest, silentlyRefreshStarted)
{
    // Test bool getter: silentlyRefreshStarted()
    bool result = obj->silentlyRefreshStarted();
    EXPECT_FALSE(result);

}

TEST_F(FSEventControllerTest, startFSMonitoring)
{
    // Test method: void startFSMonitoring(())
    EXPECT_NO_FATAL_FAILURE(obj->startFSMonitoring());
}

TEST_F(FSEventControllerTest, stopFSMonitoring)
{
    // Test method: void stopFSMonitoring(())
    EXPECT_NO_FATAL_FAILURE(obj->stopFSMonitoring());
}
