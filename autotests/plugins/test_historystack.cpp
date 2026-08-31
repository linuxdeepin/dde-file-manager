// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_historystack.cpp
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

TEST_F(HistoryStackTest, append)
{
    // Test method: void append((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->append(_arg0));
}

TEST_F(HistoryStackTest, currentIndex)
{
    // Test getter: int currentIndex()
    auto result = obj->currentIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(HistoryStackTest, forward)
{
    // Test getter: QUrl forward()
    auto result = obj->forward();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(HistoryStackTest, removeAt)
{
    // Test method: void removeAt((int i))
    EXPECT_NO_FATAL_FAILURE(obj->removeAt(0));
}

TEST_F(HistoryStackTest, removeUrl)
{
    // Test method: void removeUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeUrl(_arg0));
}

TEST_F(HistoryStackTest, size)
{
    // Test getter: int size()
    auto result = obj->size();
    EXPECT_EQ(result, 0);

}
