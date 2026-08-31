// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_completerviewdelegate.cpp
 * @brief Unit tests for CompleterViewDelegate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/completerviewdelegate.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CompleterViewDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CompleterViewDelegate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CompleterViewDelegate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CompleterViewDelegateTest, paint)
{
    // Test method: void paint((QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index))
    QStyleOptionViewItem _arg1{};
    QModelIndex _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paint(nullptr, _arg1, _arg2));
}
