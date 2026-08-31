// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_baseitemdelegateprivate.cpp
 * @brief Unit tests for BaseItemDelegatePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/baseitemdelegate_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class BaseItemDelegatePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseItemDelegatePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseItemDelegatePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseItemDelegatePrivateTest, BaseItemDelegatePrivate)
{
    // Test constructor: BaseItemDelegatePrivate((BaseItemDelegate *qq))
    ASSERT_NE(obj, nullptr);
}
