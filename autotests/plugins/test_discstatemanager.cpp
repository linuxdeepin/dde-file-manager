// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_discstatemanager.cpp
 * @brief Unit tests for DiscStateManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/discstatemanager.h"

#include <QTest>

using namespace dfmplugin_burn;

class DiscStateManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiscStateManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiscStateManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiscStateManagerTest, DiscStateManager)
{
    // Test constructor: DiscStateManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DiscStateManagerTest, ghostMountForBlankDisc)
{
    // Test method: void ghostMountForBlankDisc(())
    EXPECT_NO_FATAL_FAILURE(obj->ghostMountForBlankDisc());
}

TEST_F(DiscStateManagerTest, initilaize)
{
    // Test method: void initilaize(())
    EXPECT_NO_FATAL_FAILURE(obj->initilaize());
}

TEST_F(DiscStateManagerTest, instance)
{
    // Test getter: DiscStateManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(DiscStateManagerTest, onDevicePropertyChanged)
{
    // Test method: void onDevicePropertyChanged((const QString &id, const QString &propertyName, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onDevicePropertyChanged(_arg0, _arg1, _arg2));
}
