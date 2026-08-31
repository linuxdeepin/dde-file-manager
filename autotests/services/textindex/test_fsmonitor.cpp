// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitor.cpp
 * @brief Unit tests for FSMonitor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fsmonitor.h"

#include <QTest>

using namespace src;

class FSMonitorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSMonitor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSMonitor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSMonitorTest, FSMonitor)
{
    // Test constructor: FSMonitor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FSMonitorTest, addBlacklistedPath)
{
    // Test method: void addBlacklistedPath((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addBlacklistedPath(_arg0));
}

TEST_F(FSMonitorTest, addBlacklistedPaths)
{
    // Test method: void addBlacklistedPaths((const QStringList &paths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addBlacklistedPaths(_arg0));
}

TEST_F(FSMonitorTest, blacklistedPaths)
{
    // Test getter: QStringList blacklistedPaths()
    auto result = obj->blacklistedPaths();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FSMonitorTest, currentWatchCount)
{
    // Test getter: int currentWatchCount()
    auto result = obj->currentWatchCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FSMonitorTest, instance)
{
    // Test getter: FSMonitor instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FSMonitorTest, isActive)
{
    // Test bool getter: isActive()
    bool result = obj->isActive();
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorTest, maxAvailableWatchCount)
{
    // Test getter: int maxAvailableWatchCount()
    auto result = obj->maxAvailableWatchCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FSMonitorTest, maxResourceUsage)
{
    // Test getter: double maxResourceUsage()
    auto result = obj->maxResourceUsage();
    EXPECT_EQ(result, 0.0);

}

TEST_F(FSMonitorTest, removeBlacklistedPath)
{
    // Test method: void removeBlacklistedPath((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeBlacklistedPath(_arg0));
}

TEST_F(FSMonitorTest, setMaxResourceUsage)
{
    // Test setter: void setMaxResourceUsage((double percentage))
    EXPECT_NO_FATAL_FAILURE(obj->setMaxResourceUsage(0.0));
}

TEST_F(FSMonitorTest, setUseFastScan)
{
    // Test setter: void setUseFastScan((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setUseFastScan(false));
}

TEST_F(FSMonitorTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(FSMonitorTest, useFastScan)
{
    // Test bool getter: useFastScan()
    bool result = obj->useFastScan();
    EXPECT_FALSE(result);

}
