// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelfilter_1.cpp
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

TEST_F(CanvasModelFilterTest, insertFilter)
{
    // Test method: bool insertFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->insertFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelFilterTest, renameFilter)
{
    // Test method: bool renameFilter((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->renameFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelFilterTest, updateFilter)
{
    // Test method: bool updateFilter((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->updateFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}
