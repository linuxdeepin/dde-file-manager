// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractindexclient.cpp
 * @brief Unit tests for AbstractIndexClient methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/abstractindexclient.h"

#include <QTest>

using namespace dfmplugin_search;

class AbstractIndexClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractIndexClient();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractIndexClient *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractIndexClientTest, ensureInterface)
{
    // Test bool getter: ensureInterface()
    bool result = obj->ensureInterface();
    EXPECT_FALSE(result);

}

TEST_F(AbstractIndexClientTest, getLastUpdateTime)
{
    // Test method: void getLastUpdateTime(())
    EXPECT_NO_FATAL_FAILURE(obj->getLastUpdateTime());
}

TEST_F(AbstractIndexClientTest, startTask)
{
    // Test method: void startTask((TaskType type, const QStringList &paths, const QVariantMap &options))
    QStringList _arg1{};
    QVariantMap _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->startTask(TaskType(), _arg1, _arg2));
}
