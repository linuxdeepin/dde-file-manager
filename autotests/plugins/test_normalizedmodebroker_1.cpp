// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_normalizedmodebroker_1.cpp
 * @brief Unit tests for NormalizedModeBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalized/normalizedmodebroker.h"

#include <QTest>

using namespace ddplugin_organizer;

class NormalizedModeBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NormalizedModeBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NormalizedModeBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NormalizedModeBrokerTest, iconRect)
{
    // Test method: QRect iconRect((const QString &id, QRect vrect))
    QString _arg0{};
    auto result = obj->iconRect(_arg0, QRect());
    EXPECT_FALSE(result.isValid());

}

TEST_F(NormalizedModeBrokerTest, refreshModel)
{
    // Test method: void refreshModel((bool global, int ms, bool file))
    EXPECT_NO_FATAL_FAILURE(obj->refreshModel(false, 0, false));
}

TEST_F(NormalizedModeBrokerTest, selectAllItems)
{
    // Test bool getter: selectAllItems()
    bool result = obj->selectAllItems();
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeBrokerTest, view)
{
    // Test method: QAbstractItemView view((const QString &id))
    QString _arg0{};
    auto result = obj->view(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->view(_arg0); });

}
