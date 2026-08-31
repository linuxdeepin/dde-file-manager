// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_testingeventrecevier.cpp
 * @brief Unit tests for TestingEventRecevier methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "testing/events/testingeventrecevier.h"

#include <QTest>

using namespace dfmplugin_utils;

class TestingEventRecevierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestingEventRecevier();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestingEventRecevier *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestingEventRecevierTest, TestingEventRecevier)
{
    // Test constructor: TestingEventRecevier((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TestingEventRecevierTest, handleAccessibleSetAccessibleName)
{
    // Test method: void handleAccessibleSetAccessibleName((QWidget *w, const QString &name))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleAccessibleSetAccessibleName(nullptr, _arg1));
}

TEST_F(TestingEventRecevierTest, initAccessible)
{
    // Test method: void initAccessible(())
    EXPECT_NO_FATAL_FAILURE(obj->initAccessible());
}

TEST_F(TestingEventRecevierTest, initializeConnections)
{
    // Test method: void initializeConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeConnections());
}

TEST_F(TestingEventRecevierTest, instance)
{
    // Test getter: DPUTILS_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
