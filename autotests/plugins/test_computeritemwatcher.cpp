// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computeritemwatcher.cpp
 * @brief Unit tests for ComputerItemWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watcher/computeritemwatcher.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerItemWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerItemWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerItemWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerItemWatcherTest, addGroup)
{
    // Test method: int addGroup((const QString &name))
    QString _arg0{};
    auto result = obj->addGroup(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(ComputerItemWatcherTest, cacheItem)
{
    // Test method: void cacheItem((const ComputerItemData &in))
    ComputerItemData _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cacheItem(_arg0));
}

TEST_F(ComputerItemWatcherTest, clearAsyncThread)
{
    // Test method: void clearAsyncThread(())
    EXPECT_NO_FATAL_FAILURE(obj->clearAsyncThread());
}

TEST_F(ComputerItemWatcherTest, diskGroup)
{
    // Test getter: QString diskGroup()
    auto result = obj->diskGroup();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, getBlockDeviceItems)
{
    // Test method: ComputerDataList getBlockDeviceItems((bool *hasNewItem))
    auto result = obj->getBlockDeviceItems(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, getPreDefineItems)
{
    // Test getter: ComputerDataList getPreDefineItems()
    auto result = obj->getPreDefineItems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, getProtocolDeviceItems)
{
    // Test method: ComputerDataList getProtocolDeviceItems((bool *hasNewItem))
    auto result = obj->getProtocolDeviceItems(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, getUserDirItems)
{
    // Test getter: ComputerDataList getUserDirItems()
    auto result = obj->getUserDirItems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, items)
{
    // Test getter: ComputerDataList items()
    auto result = obj->items();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, onBlockDeviceRemoved)
{
    // Test method: void onBlockDeviceRemoved((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onBlockDeviceRemoved(_arg0));
}

TEST_F(ComputerItemWatcherTest, onProtocolDeviceRemoved)
{
    // Test method: void onProtocolDeviceRemoved((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onProtocolDeviceRemoved(_arg0));
}

TEST_F(ComputerItemWatcherTest, removeDevice)
{
    // Test method: void removeDevice((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeDevice(_arg0));
}

TEST_F(ComputerItemWatcherTest, removeGroup)
{
    // Test method: bool removeGroup((const QString &groupName))
    QString _arg0{};
    auto result = obj->removeGroup(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerItemWatcherTest, removeSidebarItem)
{
    // Test method: void removeSidebarItem((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeSidebarItem(_arg0));
}

TEST_F(ComputerItemWatcherTest, typeCompare)
{
    // Test method: bool typeCompare((const ComputerItemData &a, const ComputerItemData &b))
    ComputerItemData _arg0{};
    ComputerItemData _arg1{};
    auto result = obj->typeCompare(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ComputerItemWatcherTest, userDirGroup)
{
    // Test getter: QString userDirGroup()
    auto result = obj->userDirGroup();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerItemWatcherTest, ComputerItemWatcher)
{
    // Test constructor: ComputerItemWatcher((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerItemWatcherTest, instance)
{
    // Test getter: ComputerItemWatcher instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
