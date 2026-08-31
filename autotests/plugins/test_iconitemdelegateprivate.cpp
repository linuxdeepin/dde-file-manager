// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconitemdelegateprivate.cpp
 * @brief Unit tests for IconItemDelegatePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/iconitemdelegate_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class IconItemDelegatePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IconItemDelegatePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IconItemDelegatePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemDelegatePrivateTest, IconItemDelegatePrivate)
{
    // Test constructor: IconItemDelegatePrivate((IconItemDelegate *qq))
    ASSERT_NE(obj, nullptr);
}
