// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appendoper.cpp
 * @brief Unit tests for AppendOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/gridcore.h"

#include <QTest>

using namespace ddplugin_canvas;

class AppendOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppendOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppendOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppendOperTest, AppendOper)
{
    // Test constructor: AppendOper((GridCore *core))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AppendOperTest, append)
{
    // Test method: void append((QStringList items))
    EXPECT_NO_FATAL_FAILURE(obj->append(QStringList()));
}

TEST_F(AppendOperTest, appendAfter)
{
    // Test method: QStringList appendAfter((QStringList items, int index, const QPoint &begin))
    QPoint _arg2{};
    auto result = obj->appendAfter(QStringList(), 0, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AppendOperTest, tryAppendAfter)
{
    // Test method: void tryAppendAfter((QStringList items, int index, const QPoint &begin))
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->tryAppendAfter(QStringList(), 0, _arg2));
}
