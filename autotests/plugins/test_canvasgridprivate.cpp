// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridprivate.cpp
 * @brief Unit tests for CanvasGridPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/canvasgrid.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridPrivateTest, profiles)
{
    // Test getter: QHash<int, QHash<QString, QPoint> > profiles()
    auto result = obj->profiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridPrivateTest, sync)
{
    // Test method: void sync(())
    EXPECT_NO_FATAL_FAILURE(obj->sync());
}
