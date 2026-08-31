// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelfilter.cpp
 * @brief Unit tests for CanvasModelFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasmodelfilter.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasModelFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasModelFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasModelFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelFilterTest, CanvasModelFilter)
{
    // Test constructor: CanvasModelFilter((CanvasProxyModel *m))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasModelFilterTest, removeFilter)
{
    // Test method: bool removeFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelFilterTest, resetFilter)
{
    // Test method: bool resetFilter((QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->resetFilter(_arg0);
    EXPECT_FALSE(result);

}
