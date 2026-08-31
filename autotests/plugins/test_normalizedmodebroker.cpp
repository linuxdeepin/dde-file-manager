// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_normalizedmodebroker.cpp
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

TEST_F(NormalizedModeBrokerTest, NormalizedModeBroker)
{
    // Test constructor: NormalizedModeBroker((NormalizedMode *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(NormalizedModeBrokerTest, gridPoint)
{
    // Test method: QString gridPoint((const QUrl &item, QPoint *point))
    QUrl _arg0{};
    auto result = obj->gridPoint(_arg0, nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NormalizedModeBrokerTest, visualRect)
{
    // Test method: QRect visualRect((const QString &id, const QUrl &item))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->visualRect(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}
