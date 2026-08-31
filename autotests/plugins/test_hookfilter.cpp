// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_hookfilter.cpp
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

TEST_F(HookFilterTest, removeFilter)
{
    // Test method: bool removeFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HookFilterTest, resetFilter)
{
    // Test method: bool resetFilter((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->resetFilter(_arg0);
    EXPECT_FALSE(result);

}
