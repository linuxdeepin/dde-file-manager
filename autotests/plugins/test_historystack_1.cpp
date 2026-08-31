// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_historystack_1.cpp
 * @brief Unit tests for HistoryStack methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/historystack.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class HistoryStackTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HistoryStack();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HistoryStack *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HistoryStackTest, HistoryStack)
{
    // Test constructor: HistoryStack((int threshold))
    ASSERT_NE(obj, nullptr);
}

TEST_F(HistoryStackTest, backIsExist)
{
    // Test bool getter: backIsExist()
    bool result = obj->backIsExist();
    EXPECT_FALSE(result);

}

TEST_F(HistoryStackTest, checkPathIsExist)
{
    // Test method: bool checkPathIsExist((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->checkPathIsExist(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HistoryStackTest, forwardIsExist)
{
    // Test bool getter: forwardIsExist()
    bool result = obj->forwardIsExist();
    EXPECT_FALSE(result);

}

TEST_F(HistoryStackTest, isFirst)
{
    // Test bool getter: isFirst()
    bool result = obj->isFirst();
    EXPECT_FALSE(result);

}

TEST_F(HistoryStackTest, isLast)
{
    // Test bool getter: isLast()
    bool result = obj->isLast();
    EXPECT_FALSE(result);

}

TEST_F(HistoryStackTest, needCheckExist)
{
    // Test method: bool needCheckExist((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->needCheckExist(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HistoryStackTest, setThreshold)
{
    // Test setter: void setThreshold((int threshold))
    EXPECT_NO_FATAL_FAILURE(obj->setThreshold(0));
}
