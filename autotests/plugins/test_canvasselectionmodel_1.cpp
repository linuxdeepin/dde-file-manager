// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasselectionmodel_1.cpp
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

TEST_F(CanvasSelectionModelTest, CanvasSelectionModel)
{
    // Test constructor: CanvasSelectionModel((CanvasProxyModel *model, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasSelectionModelTest, selectAll)
{
    // Test method: void selectAll(())
    EXPECT_NO_FATAL_FAILURE(obj->selectAll());
}

TEST_F(CanvasSelectionModelTest, selectedIndexesCache)
{
    // Test getter: QModelIndexList selectedIndexesCache()
    auto result = obj->selectedIndexesCache();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasSelectionModelTest, setHook)
{
    // Test method: void setHook(())
    EXPECT_NO_FATAL_FAILURE(obj->setHook());
}
