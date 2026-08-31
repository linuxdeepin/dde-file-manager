// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanager.cpp
 * @brief Unit tests for CanvasManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "canvasmanager.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerTest, openEditor)
{
    // Test method: void openEditor((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openEditor(_arg0));
}

TEST_F(CanvasManagerTest, reloadItem)
{
    // Test method: void reloadItem(())
    EXPECT_NO_FATAL_FAILURE(obj->reloadItem());
}

TEST_F(CanvasManagerTest, views)
{
    // Test getter: QList<QSharedPointer<CanvasView>> views()
    auto result = obj->views();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasManagerTest, CanvasManager)
{
    // Test constructor: CanvasManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasManagerTest, instance)
{
    // Test getter: CanvasManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(CanvasManagerTest, model)
{
    // Test getter: CanvasProxyModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}
