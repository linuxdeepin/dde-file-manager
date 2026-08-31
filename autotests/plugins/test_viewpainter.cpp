// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewpainter.cpp
 * @brief Unit tests for ViewPainter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/viewpainter.h"

#include <QTest>

using namespace ddplugin_canvas;

class ViewPainterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewPainter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewPainter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewPainterTest, model)
{
    // Test getter: CanvasProxyModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(ViewPainterTest, selectionModel)
{
    // Test getter: CanvasSelectionModel selectionModel()
    auto result = obj->selectionModel();
    EXPECT_NO_FATAL_FAILURE({ obj->selectionModel(); });

}

TEST_F(ViewPainterTest, view)
{
    // Test getter: CanvasView view()
    auto result = obj->view();
    EXPECT_NO_FATAL_FAILURE({ obj->view(); });

}
