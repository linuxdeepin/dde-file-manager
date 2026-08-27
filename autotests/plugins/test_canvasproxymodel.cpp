// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasproxymodel.cpp
 * @brief Unit tests for CanvasProxyModel Mid-priority methods (ddplugin-canvas)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "model/canvasproxymodel.h"

using namespace ddplugin_canvas;

class CanvasProxyModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(CanvasProxyModelTest, data)
{
    // Instance method data
    CanvasProxyModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.data(QModelIndex(), 0); (void)r; });
}

TEST_F(CanvasProxyModelTest, fileUrl)
{
    // Instance method fileUrl
    CanvasProxyModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.fileUrl(QModelIndex()); (void)r; });
}

TEST_F(CanvasProxyModelTest, files)
{
    // Instance method files
    CanvasProxyModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.files(); (void)r; });
}

TEST_F(CanvasProxyModelTest, index)
{
    // Instance method index
    CanvasProxyModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.index(0, 0, QModelIndex()); (void)r; });
}

TEST_F(CanvasProxyModelTest, parent)
{
    // Instance method parent
    CanvasProxyModel obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.parent(QModelIndex()); (void)r; });
}

TEST_F(CanvasProxyModelTest, setSourceModel)
{
    // Instance method setSourceModel
    CanvasProxyModel obj;
    EXPECT_NO_FATAL_FAILURE({ obj.setSourceModel(nullptr); });
}

TEST_F(CanvasProxyModelTest, sort)
{
    // Instance method sort
    CanvasProxyModel obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.sort(); });
    (void)result;
}

TEST_F(CanvasProxyModelTest, take)
{
    // Instance method take
    CanvasProxyModel obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.take(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(CanvasProxyModelTest, CanvasProxyModel)
{
    // CanvasProxyModel
    SUCCEED();
}
