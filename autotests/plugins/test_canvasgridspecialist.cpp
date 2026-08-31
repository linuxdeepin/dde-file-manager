// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridspecialist.cpp
 * @brief Unit tests for CanvasGridSpecialist methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/canvasgridspecialist.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridSpecialistTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridSpecialist();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridSpecialist *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridSpecialistTest, sortItemInGrid)
{
    // Test method: QStringList sortItemInGrid((const QHash<QString, QPoint> &items))
    QHash<QString, QPoint> _arg0{};
    auto result = obj->sortItemInGrid(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
