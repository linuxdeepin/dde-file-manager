// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_hookfilter_1.cpp
 * @brief Unit tests for HookFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasmodelfilter.h"

#include <QTest>

using namespace ddplugin_canvas;

class HookFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HookFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HookFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HookFilterTest, insertFilter)
{
    // Test method: bool insertFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->insertFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HookFilterTest, renameFilter)
{
    // Test method: bool renameFilter((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->renameFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(HookFilterTest, updateFilter)
{
    // Test method: bool updateFilter((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->updateFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}
