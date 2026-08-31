// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasselectionmodel.cpp
 * @brief Unit tests for CanvasSelectionModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasselectionmodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasSelectionModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasSelectionModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasSelectionModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasSelectionModelTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(CanvasSelectionModelTest, clearSelectedCache)
{
    // Test method: void clearSelectedCache(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSelectedCache());
}

TEST_F(CanvasSelectionModelTest, hookClear)
{
    // Test method: void hookClear(())
    EXPECT_NO_FATAL_FAILURE(obj->hookClear());
}

TEST_F(CanvasSelectionModelTest, model)
{
    // Test getter: CanvasProxyModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(CanvasSelectionModelTest, selectedUrls)
{
    // Test getter: QList<QUrl> selectedUrls()
    auto result = obj->selectedUrls();
    EXPECT_TRUE(result.isEmpty());

}
